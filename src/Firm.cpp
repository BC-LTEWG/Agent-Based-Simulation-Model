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
        recorded_living_labor_per_unit[product] = product->global_living_labor_per_unit;
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
    log_shipment_received(order->product, order->quantity);
    log_inventory_level(order->product, input_inventory[order->product]);

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
    log_inventory_reduction(product, quantity);
    log_inventory_level(product, input_inventory[product]);
    return true;
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
        if (static_cast<int>(transfers.size()) == max_workers_to_transfer) break;
        transfers.push_back(worker);
    }
    return transfers;
}

void Firm::finalize_transfer(Person * worker) {
    standby_workers.erase(worker);
    workers.erase(worker);
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
        log_reorder(product, reorder_quantity);
        log_accepted_order(product->product_name, order->requested_turnaround_time);
    } else {
        log_reorder_failure(product, reorder_quantity);
    }
}

void Firm::check_and_reorder_inputs() {
    for (std::pair<Product *, int> stockpile : input_inventory) {
        check_and_reorder_input(stockpile.first);
    }
}

void Firm::check_and_reorder_input(Product * product) {
    double threshold = get_reorder_threshold(product);
    log_demand(product->product_name, threshold);
    int pending_inventory = get_pending_input_inventory(product);
    log_pending_inventory(product->product_name, pending_inventory);
    if (pending_inventory < threshold) {
        reorder_input_product_to_threshold(product, threshold, pending_inventory);
    }
}

int Firm::predict_workers_needed(Plan * plan) {
    return std::ceil(
            plan->order->quantity *
            plan->order->product->living_labor_per_unit *
            WEEK /
            Sim::get_work_days_weekly() / 
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
    for (Person * worker : sorted_standby_workers) {
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
            Sim::get_work_days_weekly() / 
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

int Firm::calculate_raw_material_cost_for_order(Order * order) {
    int raw_material_cost = 0;
    for (std::pair<Product * const, double>& input : order->product->inputs_per_unit) {
        raw_material_cost += input.first->price_per_unit *
            input.second *
            order->quantity;
    }
    return raw_material_cost;
}

void Firm::initialize_plan_budget(
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
        predict_turnaround_time(draft_plan, draft_plan->workers);
    draft_plan->labor_hours = 
        draft_plan->labor_hours_remaining =
        predict_labor_hours(draft_plan->order, draft_plan->workers); 
    initialize_plan_budget(draft_plan);
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

void Firm::log_shipment_received(const Product * product, const int quantity) {
    log_product_quantity("shipment_received", product, quantity);
}

void Firm::log_inventory_level(const Product * product, const int quantity) {
    log_product_quantity("inventory_level", product, quantity);
}

void Firm::log_inventory_reduction(const Product * product, const int quantity) {
    log_product_quantity("inventory_reduction", product, quantity);
}

void Firm::log_reorder(const Product * product, const int quantity) {
    log_product_quantity("reorder", product, quantity);
}

void Firm::log_reorder_failure(const Product * product, const int quantity) {
    log_product_quantity("reorder_failure", product, quantity);
}

void Firm::log_product_quantity(
        const char * const label,
        const Product * product,
        const int quantity
        ) {
    Logger::get_instance()->log<int>(
            get_client_type(),
            label,
            id,
            "product_id",
            product->id,
            "amount",
            quantity
            );
}

void Firm::log_accepted_order(std::string product_name, int requested_turnaround_time) {
    Logger::get_instance()->log(
            get_client_type(),
            "accepted_order",
            id,
            product_name,
            requested_turnaround_time
            );
}

void Firm::log_demand(std::string product_name, double demand) {
    Logger::get_instance()->log(
            get_client_type(),
            "current_demand",
            id,
            product_name,
            demand*FIRM_STOCKPILE_DURATION
            );
}

void Firm::log_pending_inventory(std::string product_name, double pending_inventory) {
    Logger::get_instance()->log(
            get_client_type(),
            "pending_inventory",
            id,
            product_name,
            pending_inventory
            );
}

void Firm::log_input_inventory(Firm * firm, std::string product_name, int quantity) {
    Logger::get_instance()->log(
            get_client_type(),
            "input_inventory",
            firm->get_id(),
            product_name,
            quantity
            );
}
