#include <algorithm>
#include <climits>
#include <iostream>
#include <numeric>

#include "Constants.h"
#include "Distributor.h"
#include "Firm.h"
#include "Logger.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Order::Order(
        Product * product,
        int quantity,
        Firm * customer,
        int requested_turnaround_time
        )
    : product(product),
      quantity(quantity),
      customer(customer),
      requested_turnaround_time(requested_turnaround_time),
      status(ORDER_REQUESTED)
{}

Firm::Firm(Society * society) : society{society} {}

Firm::Firm(Society * society, std::unordered_set<Product *> initial_catalog, std::unordered_map<Product *, int>& input_inventory) :
    society{society},
    catalog(initial_catalog),
    inventory(input_inventory)
{
    static unsigned int unique_id = 0;
    id = unique_id++;
}

unsigned int Firm::get_id() {
    return id;
}

void Firm::on_time_step() {
    apply_demand_window();
    check_and_reorder_inputs();
}

void Firm::initialize_inventory(
        std::unordered_map<Product *, int>& inventory_items
        ) {
    for(auto& items : inventory_items) {
        inventory[items.first] = items.second;
    }
}

double Firm::get_avg_productivity() {
    return 0.0;
}

int Firm::get_inventory(Product * product) {
    std::unordered_map<Product *, int>::iterator it = inventory.find(product);
    if (it == inventory.end()) {
        return 0;
    }
    return inventory[product];
}

void Firm::add_supplier(Producer * producer) {
    suppliers.push_back(producer);
}

void Firm::receive_shipment(Order * order) {
    if (order->status != Order::ORDER_FINISHED) {
        std::cerr << "Attempted to recieve a shipment for an incomplete order."
            << std::endl;
        return;
    }
    inventory[order->product] += order->quantity;
    product_to_pending_inbound_orders[order->product].erase(order);
    log_shipment_received(order->product->product_name, order->quantity);
    log_inventory_level(order->product->product_name, inventory[order->product]);
}

Producer * Firm::send_order(Order * order) {
    int order_time = INT_MAX;
    Producer * chosen_producer = nullptr;

    for (Producer * producer : suppliers) {
        int draft_plan_time = producer->draft_plan(order);
        if (draft_plan_time != DRAFT_ORDER_REJECTED &&
                draft_plan_time < order_time) {
            order_time = draft_plan_time;
            chosen_producer = producer;
        }
    }
    if (chosen_producer) {
        chosen_producer->pursue_order(order);
        product_to_pending_inbound_orders[order->product].insert(order);
    }
    for (auto * producer : suppliers) {
        if (producer != chosen_producer) {
            producer->drop_order(order);
        }
    }

    return chosen_producer;
}

double Firm::get_reorder_threshold(Product * product) {
    return std::max((double) product->order_size,
        inventory_demands[product] * FIRM_STOCKPILE_DURATION);
}

int Firm::get_pending_input_inventory(Product * product) {
    int pending_inventory = inventory[product];
    for (Order * order : product_to_pending_inbound_orders[product]) {
        pending_inventory += order->quantity;
    }
    return pending_inventory;
}

void Firm::reorder_input_product_to_threshold(
        Product * product,
        double threshold,
        int pending_inventory
        ) {
    if (pending_inventory < threshold) {
        int reorder_quantity = static_cast<int>(std::ceil(threshold));
        int reorder_deadline = static_cast<int>(
            pending_inventory *
            FIRM_STOCKPILE_DURATION *
            DEADLINE_SAFETY_MULTIPLIER /
            threshold
            );
        log_reorder(product->product_name, reorder_quantity);
        Order * order = new Order(
                product,
                reorder_quantity,
                this,
                reorder_deadline
                );
        Producer * chosen_producer = send_order(order);
        if (chosen_producer) {
            log_accepted_order(product->product_name, order->requested_turnaround_time);
        }
    }
}

void Firm::check_and_reorder_inputs() {
    std::unordered_set<Product *> input_products_to_reorder =
        get_products_to_reorder();
    for (Product * product : input_products_to_reorder) {
        double threshold = get_reorder_threshold(product);
        int pending_inventory = get_pending_input_inventory(product);
        if (pending_inventory < threshold) {
            reorder_input_product_to_threshold(product, threshold, pending_inventory);
        }
    }
}

double Firm::suitability(
        Person * person,
        std::vector<Person::Ability>& required_abilities
        ) {
    return suitability(person->get_abilities(),
            required_abilities,
            person->get_current_productivity());
}

double Firm::suitability(
        std::unordered_map<Person::Ability, double>& abilities,
        std::vector<Person::Ability>& required_abilities,
        float productivity
        ) {
    double suitability = 0.0;
    for (Person::Ability ability : required_abilities) {
        suitability += abilities[ability];
    }
    suitability /= required_abilities.size();
    suitability *= productivity;
    return suitability;
}

int Firm::predict_workers_needed(Order * order) {
    return ceil(
            order->quantity *
            order->product->living_labor_per_unit *
            DAY /
            society->get_current_work_hours_daily() /
            order->requested_turnaround_time
            );
}

void Firm::assign_workers_by_suitability_threshold(
        Plan * draft_plan,
        std::vector<Person::Ability>& required_abilities,
        double suitability_threshold
        ) {
    std::unordered_map<Person *, double> worker_to_suitability;
    for (Person * worker : workers) {
        worker_to_suitability[worker] =
            suitability(worker, required_abilities);
    }
    std::sort(workers.begin(), workers.end(), [&](Person * a, Person * b) {
            return worker_to_suitability[a] > worker_to_suitability[b];
            });

    std::size_t max_workers = predict_workers_needed(draft_plan->order);
    for (Person * worker : workers) {
        if (
                draft_plan->workers.size() >= max_workers ||
                worker_to_suitability[worker] <= suitability_threshold
           ) {
            break;
        } 
        draft_plan->workers.push_back(worker);
    }
    for (
            Person * unemployed_person :
            society->get_unemployed_people()
        ) {    
        if (draft_plan->workers.size() >= max_workers) {
            break;
        } else if (
                suitability(unemployed_person, required_abilities) >
                suitability_threshold
                ) {
            draft_plan->workers.push_back(unemployed_person);
        }
    }
}


int Firm::predict_turnaround_time(Order * order, double total_suitability) {
    return std::ceil(
            order->quantity *
            order->product->living_labor_per_unit *
            DAY /
            total_suitability /
            society->get_current_work_hours_daily()
            );
}

int Firm::predict_labor_hours(Order * order, double total_suitability) {
    return std::ceil(
            order->quantity *
            order->product->living_labor_per_unit /
            total_suitability
            );
}

void Firm::assign_plan_dependent_fields(
        Plan * draft_plan,
        std::vector<Person::Ability>& required_abilities
        ) {
    double total_suitability = 0.0;
    if (draft_plan->training_time) {
        std::unordered_map<Person::Ability, double> max_required_abilities;
        for (Person * worker : draft_plan->workers) {
            for (Person::Ability ability : required_abilities) {
                max_required_abilities[ability] =
                    std::max(
                            max_required_abilities[ability],
                            worker->get_abilities()[ability]
                            );
            }
        }
        for (Person * worker : draft_plan->workers) {
            total_suitability += suitability(max_required_abilities,
                    required_abilities,
                    worker->get_current_productivity());
        }
        if (total_suitability <= 0) {
            total_suitability = 1.0;
        }
        draft_plan->predicted_turnaround_time =
            draft_plan->training_time +
            predict_turnaround_time(draft_plan->order, total_suitability);
        draft_plan->labor_hours =
            draft_plan->labor_hours_remaining =
            draft_plan->training_time * draft_plan->workers.size() +
            predict_labor_hours(draft_plan->order, total_suitability);
    } else {
        for (Person * worker : draft_plan->workers) {
            total_suitability += suitability(worker, required_abilities);
        }
        if (total_suitability <= 0) {
            total_suitability = 1.0;
        }
        draft_plan->predicted_turnaround_time =
            predict_turnaround_time(draft_plan->order, total_suitability);
        draft_plan->labor_hours =
            draft_plan->labor_hours_remaining =
            predict_labor_hours(draft_plan->order, total_suitability);
    }

    int raw_materials = 0;
    for (
            std::pair<Product * const, double>& p :
            draft_plan->order->product->inputs_per_unit
            ) {
        raw_materials += p.first->price_per_unit *
            p.second *
            draft_plan->order->quantity;
    }
    draft_plan->raw_materials = 
        draft_plan->raw_materials_remaining = raw_materials;
    draft_plan->total_hours =
        draft_plan->total_hours_remaining =
        draft_plan->labor_hours + draft_plan->raw_materials;
    draft_plan->prd = -(draft_plan->total_hours);
}

void Firm::draft_optimal_plan(
        Plan * draft_plan,
        std::vector<Person::Ability>& required_abilities
        ) {
    // try without training first
    Plan * draft_plan_without_training = new Plan(*draft_plan);
    assign_workers_by_suitability_threshold(
            draft_plan_without_training,
            required_abilities,
            0.0
            );
    assign_plan_dependent_fields(
            draft_plan_without_training,
            required_abilities
            );
    if (
            draft_plan_without_training->predicted_turnaround_time <=
            draft_plan->order->requested_turnaround_time
       ) {
        *draft_plan = *draft_plan_without_training; return;
    }

    // did not meet the deadline without training
    // pick the better option between training or no training
    Plan * draft_plan_with_training = new Plan(*draft_plan_without_training);
    draft_plan_with_training->training_time =
        draft_plan_with_training->training_time_remaining = FIRM_TRAINING_TIME;
    assign_plan_dependent_fields(draft_plan_with_training, required_abilities);
    if (
            draft_plan_without_training->predicted_turnaround_time <
            draft_plan_with_training->predicted_turnaround_time
       ) {
        *draft_plan = *draft_plan_without_training;
    } else {
        *draft_plan = *draft_plan_with_training;
    }
}

void Firm::train_workers(
        std::vector<Person *>& workers,
        std::vector<Person::Ability>& required_abilities
        ) {
    std::unordered_map<Person::Ability, double> max_required_abilities;
    for (Person * worker : workers) {
        for (Person::Ability ability : required_abilities) {
            max_required_abilities[ability] =
                std::max(
                        max_required_abilities[ability],
                        worker->get_abilities()[ability]
                        );
        }
    }
    for (Person * worker : workers) {
        worker->train(max_required_abilities);    
    }
}

void Firm::add_demand_signal(Product * product, int quantity) {
    demand_signals.push({product, quantity, Sim::get_current_time_step()});
    inventory_demands[product] += (double) quantity / FIRM_DEMAND_WINDOW;
}

void Firm::apply_demand_window() {
    while (!demand_signals.empty() && 
            demand_signals.front().timestep <= 
            Sim::get_current_time_step() - FIRM_DEMAND_WINDOW) {
        inventory_demands[demand_signals.front().product] -= 
            (double) demand_signals.front().quantity / FIRM_DEMAND_WINDOW;
        demand_signals.pop();
    }
}

void Firm::log_shipment_received(std::string product_name, int quantity) {
    Logger::get_instance()->log(
            Logger::FIRM,
            "shipment_received",
            id,
            product_name,
            quantity
            );
}

void Firm::log_inventory_level(std::string product_name, int quantity) {
    Logger::get_instance()->log(
            Logger::FIRM,
            "inventory_level",
            id,
            product_name,
            quantity
            );
}

void Firm::log_reorder(std::string product_name, int quantity) {
    Logger::get_instance()->log(
            Logger::FIRM,
            "reorder",
            id,
            product_name,
            quantity
            );
}

void Firm::log_accepted_order(std::string product_name, int requested_turnaround_time) {
    Logger::get_instance()->log(
            Logger::FIRM,
            "accepted_order",
            id,
            product_name,
            requested_turnaround_time
            );
}
