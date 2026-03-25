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

Firm::Firm(Society * society, std::unordered_set<Product *> initial_catalog) :
    society{society},
    catalog(initial_catalog)
{
    static unsigned int unique_id = 0;
    id = unique_id++;
    for (Product * product : society->get_products()) {
        recorded_living_labor_per_unit[product] = product->living_labor_per_unit;
    }
}

unsigned int Firm::get_id() {
    return id;
}

void Firm::on_time_step() {
    apply_demand_window();
    check_and_reorder();
    for (Product * product : get_products_to_reorder()) {
        log_demand(product->product_name, get_demand(product));
    }
}

void Firm::initialize_inventory(
        std::unordered_map<Product *, int>& inventory_items
        ) {
    for(auto& items : inventory_items) {
        inventory[items.first] = items.second;
    }
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
    product_to_outbound_orders[order->product].erase(order);
    log_shipment_received(order->product->product_name, order->quantity);
}

double Firm::get_busyness() {
    double busyness = 0.0;
    for (Person * worker : workers) {
        busyness += worker->get_busyness();
    }
    return workers.size() > 0 ? busyness / workers.size() : 0.0;
}

std::vector<Person *> Firm::propose_transfer(int workers_wanted) {
    if (get_busyness() >= society->get_busyness() - TRANSFER_BUSYNESS_THRESHOLD) {
        return {};
    }
    int max_workers_to_transfer = (int) (workers.size() * (1.0 - get_busyness() / 
            (society->get_busyness() - TRANSFER_BUSYNESS_THRESHOLD))); 
    max_workers_to_transfer = std::max(max_workers_to_transfer, workers_wanted);
    std::vector<Person *> transfers;
    for (Person * worker : standby_workers) {
        if (transfers.size() == max_workers_to_transfer) break;
        transfers.push_back(worker);
    }
    return transfers;
}

void Firm::finalize_transfer(Person * worker) {
    standby_workers.erase(worker);
    workers.erase(worker);
}

Producer * Firm::send_order(Order * order) {
    int order_time = INT_MAX;
    Producer * chosen_producer = nullptr;

    std::vector<Producer *> primary_producers, 
        secondary_producers,
        rejecting_primary_producers;
    for (Producer * producer : suppliers) {
        if (producer->can_produce(order->product)) {
            primary_producers.push_back(producer);
        } else {
            secondary_producers.push_back(producer);
        }
    }
    for (Producer * producer : primary_producers) {
        int draft_plan_time = producer->draft_plan(order);
        if (draft_plan_time == DRAFT_ORDER_REJECTED) {
            producer->drop_order(order);
            rejecting_primary_producers.push_back(producer);
            continue;
        }
        if (draft_plan_time < order_time) {
            if (chosen_producer) {
                chosen_producer->drop_order(order);
            }
            order_time = draft_plan_time;
            chosen_producer = producer;
        }
    }
    if (!chosen_producer) {
        for (Producer * producer : secondary_producers) {
            int draft_plan_time = producer->draft_plan(order);
            if (draft_plan_time == DRAFT_ORDER_REJECTED) {
                producer->drop_order(order);
                continue;
            }
            order_time = draft_plan_time;
            chosen_producer = producer;
            break;
        }
    }
    if (chosen_producer) {
        chosen_producer->pursue_order(order);
        product_to_outbound_orders[order->product].insert(order);
    }
    return chosen_producer;
}

double Firm::get_reorder_threshold(Product * product) {
    return std::max((double) product->order_size,
        get_demand(product) * FIRM_STOCKPILE_DURATION);
}

int Firm::get_pending_output_inventory(Product * product) {
    int pending_inventory = inventory[product];
    for (Order * order : product_to_outbound_orders[product]) {
        pending_inventory += order->quantity;
    }
    return pending_inventory;
}

void Firm::reorder_output_product_to_threshold(
        Product * product,
        double threshold,
        int pending_inventory
        ) {
    if (pending_inventory < threshold) {
        int reorder_quantity = static_cast<int>(std::ceil(threshold));
        int reorder_deadline = static_cast<int>(
            pending_inventory *
            FIRM_STOCKPILE_DURATION *
            DEADLINE_SAFETY_MULT /
            threshold
            );
        Order * order = new Order(
                product,
                reorder_quantity,
                this,
                reorder_deadline
                );
        Producer * chosen_producer = send_order(order);
        if (chosen_producer) {
            log_reorder(product->product_name, reorder_quantity);
            log_accepted_order(product->product_name, order->requested_turnaround_time);
        } else {
            log_reorder("No producer found for " + product->product_name, reorder_quantity);
        }
    }
}

void Firm::check_and_reorder() {
    std::unordered_set<Product *> products_to_reorder =
        get_products_to_reorder();
    for (Product * product : products_to_reorder) {
        double threshold = get_reorder_threshold(product);
        int pending_inventory = get_pending_output_inventory(product);
        if (pending_inventory < threshold) {
            reorder_output_product_to_threshold(product, threshold, pending_inventory);
        }
    }
}

int Firm::predict_workers_needed(Plan * plan) {
    return std::ceil(
            plan->order->quantity *
            plan->order->product->global_living_labor_per_unit *
            WEEK /
            INITIAL_WORK_DAYS_WEEKLY / 
            plan->local_work_hours_daily /
            plan->order->requested_turnaround_time
            );
}

void Firm::assign_workers(
        Plan * draft_plan,
        std::vector<Person::Ability>& required_abilities
        ) {
    std::vector<Person *> sorted_standby_workers(standby_workers.begin(),
            standby_workers.end());
    std::sort(sorted_standby_workers.begin(), sorted_standby_workers.end(), 
            [&](Person * a, Person * b) {
            return a->get_busyness() < b->get_busyness();
            });

    int workers_left = predict_workers_needed(draft_plan);
    for (Person * worker : standby_workers) {
        if (workers_left == 0) return;
        draft_plan->workers.push_back(worker);
        workers_left--;
    }
    for (Person * unemployed_person : society->get_unemployed_people()) {
        if (workers_left == 0) return;
        draft_plan->workers.push_back(unemployed_person);
        workers_left--;
    }
    for (Producer * producer : society->get_producers()) {
        if (workers_left == 0) return;
        if (producer == this) continue;
        std::vector<Person *> transfers = producer->propose_transfer(workers_left);
        for (Person * transfer : transfers) {
            draft_plan->workers.push_back(transfer);
        }
        workers_left -= transfers.size();
    }
}

int Firm::predict_turnaround_time(Plan * plan, std::vector<Person *>& workers) {
    return std::ceil(
            plan->order->quantity *
            recorded_living_labor_per_unit[plan->order->product] *
            WEEK /
            INITIAL_WORK_DAYS_WEEKLY / 
            plan->local_work_hours_daily /
            workers.size()
            );
}

int Firm::predict_labor_hours(Order * order, std::vector<Person *>& workers) {
    return std::ceil(
            order->quantity *
            recorded_living_labor_per_unit[order->product] / 
            workers.size()
            );
}

void Firm::assign_plan_dependent_fields(
        Plan * draft_plan,
        std::vector<Person::Ability>& required_abilities
        ) {
    draft_plan->predicted_turnaround_time =
        predict_turnaround_time(draft_plan, draft_plan->workers);
    draft_plan->labor_hours = 
        draft_plan->labor_hours_remaining =
        predict_labor_hours(draft_plan->order, draft_plan->workers); 
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
    draft_plan->quantity_remaining = draft_plan->order->quantity;
    draft_plan->prd = -(draft_plan->total_hours);
}

void Firm::draft_optimal_plan(
        Plan * draft_plan,
        std::vector<Person::Ability>& required_abilities
        ) {
    // try without training first
    Plan * draft_plan_without_training = new Plan(*draft_plan);
    assign_workers(
        draft_plan_without_training,
        required_abilities
    );
    assign_plan_dependent_fields(
        draft_plan_without_training,
        required_abilities
    );
    *draft_plan = *draft_plan_without_training;
}

void Firm::train_workers(
        std::vector<Person *>& workers,
        std::vector<Person::Ability>& required_abilities
        ) {
    // training has to be revised.
}

void Firm::add_demand_signal(Product * product, int quantity) {
    demand_signals[product].push({quantity, Sim::get_current_time_step()});
    total_demands[product] += quantity;
}

void Firm::apply_demand_window() {
    for (std::pair<Product * const, std::queue<DemandSignal>>& product : demand_signals) {
        std::queue<DemandSignal>& signals = product.second;
        while (!signals.empty() && 
                signals.front().timestep <= 
                Sim::get_current_time_step() - FIRM_DEMAND_WINDOW_MAX) {
            total_demands[product.first] -= 
                signals.front().quantity;
            signals.pop();
        }
    }
}

double Firm::get_demand(Product * product) {
    int window_start = Sim::get_current_time_step();
    if (!demand_signals[product].empty()) {
        window_start = demand_signals[product].front().timestep;
    }
    int window_length = std::max(FIRM_DEMAND_WINDOW_MIN, 
        Sim::get_current_time_step() - window_start);
    return (double) total_demands[product] / window_length;
}

void Firm::move_worker_off_standby(Person * worker) {
    if (worker->get_firm() == nullptr) {
        society->get_unemployed_people().erase(worker);
    } else if (worker->get_firm() == this) {
        standby_workers.erase(worker);
    } else {
        worker->get_firm()->finalize_transfer(worker);
    }
    worker->set_firm(this);
    workers.insert(worker);
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

void Firm::log_demand(std::string product_name, double demand) {
    Logger::get_instance()->log(
            Logger::FIRM,
            "demand " + product_name,
            id,
            product_name,
            demand
            );
}
