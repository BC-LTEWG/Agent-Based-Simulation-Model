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

Firm::Firm(
        Society * society,
        const std::unordered_set<Product *>& initial_catalog
        ) :
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
}

int Firm::get_inventory(Product * product) {
    std::unordered_map<Product *, int>::iterator it = input_inventory.find(product);
    if (it == input_inventory.end()) {
        return 0;
    }
    return input_inventory[product];
}

void Firm::add_supplier(Producer * producer) {
    suppliers.push_back(producer);
}

void Firm::receive_shipment(Plan * plan) {
    Order * order = plan->order;
    if (order->status != Order::ORDER_FINISHED) {
        std::cerr << "Attempted to recieve a shipment for an incomplete order."
            << std::endl;
        return;
    }
    input_inventory[order->product] += order->quantity;
    product_to_outbound_orders[order->product].erase(order);
    int transaction_amount = order->product->price_per_unit * order->quantity;
    pooled_input_value_account -= transaction_amount;
    plan->firm->receive_payment(plan, transaction_amount);
    log_shipment_received(order->product->product_name, order->quantity);
    log_inventory_level(order->product->product_name, input_inventory[order->product]);

}

void Firm::receive_payment(Plan * plan, int transaction_amount) {
    plan->prd += transaction_amount;

}

bool Firm::remove_input_from_inventory(Product * product, int quantity) {
    if (input_inventory[product] < quantity) {
        return false;
        std::cerr << "No good to remove from" << std::endl;
    }
    input_inventory[product] -= quantity;
    return true;
}

Producer * Firm::send_order(Order * order) {
    Producer * chosen_producer = select_fastest_supplier_for_order(order);
    if (chosen_producer) {
        pursue_order_with_chosen_producer(order, chosen_producer);
    }
    drop_order_from_unchosen_producer(order, chosen_producer);
    return chosen_producer;
}

Producer * Firm::select_fastest_supplier_for_order(Order * order) {
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
    return chosen_producer;
}

void Firm::pursue_order_with_chosen_producer(
        Order * order,
        Producer * chosen_producer
        ) {
    chosen_producer->pursue_order(order);
    chosen_producer->plans_in_progress.back()->prd +=
        order->product->price_per_unit * order->quantity;
    product_to_outbound_orders[order->product].insert(order);
}

void Firm::drop_order_from_unchosen_producer(
        Order * order,
        Producer * unchosen_producer
        ) {
    for (Producer * producer : suppliers) {
        if (producer != unchosen_producer) {
            producer->drop_order(order);
        }
    }
}

double Firm::get_reorder_threshold(Product * product) {
    return std::max((double) product->order_size,
        get_demand(product) * FIRM_STOCKPILE_DURATION);
}

int Firm::get_pending_input_inventory(Product * product) {
    int pending_inventory = input_inventory[product];
    for (Order * order : product_to_outbound_orders[product]) {
        pending_inventory += order->quantity;
    }
    return pending_inventory;
}

void Firm::reorder_input_product_to_threshold(
        Product * product,
        double threshold,
        int pending_inventory
        ) {
    if (pending_inventory >= threshold) {
        std::cerr << "Reordering product " << product->product_name
            << " unnecessarily." << std::endl;
        return;
    }

    int reorder_quantity = static_cast<int>(
        std::ceil(threshold - pending_inventory)
        );
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

void Firm::check_and_reorder_inputs() {
    for (std::pair<Product *, int> stockpile : input_inventory) {
        check_and_reorder_input(stockpile.first);
    }
}

void Firm::check_and_reorder_input(Product * product) {
    double threshold = get_reorder_threshold(product);
    int pending_inventory = get_pending_input_inventory(product);
    if (pending_inventory < threshold) {
        reorder_input_product_to_threshold(product, threshold, pending_inventory);
    }
}

int Firm::predict_workers_needed(Order * order) {
    return std::ceil(
            order->quantity *
            order->product->global_living_labor_per_unit *
            DAY /
            society->get_current_work_hours_daily() /
            order->requested_turnaround_time
            );
}

void Firm::assign_workers(
        Plan * draft_plan,
        std::vector<Person::Ability>& required_abilities
        ) {
    std::sort(workers.begin(), workers.end(), [&](Person * a, Person * b) {
            return a->suitability(required_abilities) > b->suitability(required_abilities);
            });

    int workers_left = predict_workers_needed(draft_plan->order);
    for (unsigned int i = 0; i < workers.size() && workers_left > 0; ++i) {
        draft_plan->workers.push_back(workers[i]);
        workers_left--;
    }
    for (unsigned int i = 0; i < society->get_unemployed_people().size() && workers_left > 0; i++) {
        draft_plan->workers.push_back(society->get_unemployed_people()[i]);
        workers_left--;
    }
}

int Firm::predict_turnaround_time(Order * order, std::vector<Person *>& workers) {
    return std::ceil(
            order->quantity *
            recorded_living_labor_per_unit[order->product] *
            DAY /
            society->get_current_work_hours_daily() / 
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

int Firm::calculate_raw_material_cost_for_order(Order * order) {
    int raw_material_cost = 0;
    for (std::pair<Product * const, double>& input : order->product->inputs_per_unit) {
        raw_material_cost += input.first->price_per_unit *
            input.second *
            order->quantity;
    }
    return raw_material_cost;
}

void Firm::initialize_plan_progress_metrics(
        Plan * draft_plan
        ) {
    int raw_material_cost = calculate_raw_material_cost_for_order(draft_plan->order);
    draft_plan->raw_materials =
        draft_plan->raw_materials_remaining = raw_material_cost;
    draft_plan->total_hours =
        draft_plan->total_hours_remaining =
        draft_plan->labor_hours + draft_plan->raw_materials;
    draft_plan->quantity_remaining = draft_plan->order->quantity;
    draft_plan->prd = -(draft_plan->total_hours);
}

void Firm::assign_plan_dependent_fields(
        Plan * draft_plan,
        std::vector<Person::Ability>& required_abilities
        ) {
    draft_plan->predicted_turnaround_time =
        predict_turnaround_time(draft_plan->order, draft_plan->workers);
    draft_plan->labor_hours = 
        draft_plan->labor_hours_remaining =
        predict_labor_hours(draft_plan->order, draft_plan->workers); 
    initialize_plan_progress_metrics(draft_plan);
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

void Firm::log_input_inventory(Firm * firm, std::string product_name, int quantity) {
    Logger::get_instance()->log(
            Logger::FIRM,
            "input_inventory",
            firm->get_id(),
            product_name,
            quantity
            );
}
