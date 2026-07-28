#include <algorithm>
#include <climits>
#include <iostream>
#include <numeric>
#include <sstream>

#include "Constants.h"
#include "ConsumerGood.h"
#include "Distributor.h"
#include "Firm.h"
#include "Good.h"
#include "Logger.h"
#include "Machine.h"
#include "Order.h"
#include "Person.h"
#include "Plan.h"
#include "PriceController.h"
#include "Producer.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Firm::Firm() {
    static unsigned int unique_id = 0;
    id = unique_id++;
    for (Product * product : Society::get_instance()->get_products()) {
        recorded_living_labor_per_unit[product] = product->living_labor_per_unit;
    }
}

unsigned int Firm::get_id() {
    return id;
}

void Firm::on_time_step() {
    update_demands();
    std::unordered_set<Product *> products_to_check;
    for (std::pair<Product * const, double>& demand : reproduction_demands) {
        products_to_check.insert(demand.first);
    }
    for (std::pair<Product * const, double>& demand : consumer_demands) {
        products_to_check.insert(demand.first);
    }
    for (Product * product : products_to_check) {
        check_and_reorder_input(product);
    }
    move_plans_forward_one_step();
    if (plans_in_progress.size()) {
        log_plans();
    }
}

double Firm::get_inventory_level(Product * product) {
    return input_inventory.count(product) ? input_inventory[product] : 0;
}

void Firm::receive_shipment(Plan * plan) {
    Order * order = plan->order;
    double quantity_delivered = order->quantity - plan->quantity_remaining;
    input_inventory[order->product] += quantity_delivered;
    product_to_outbound_orders[order->product].erase(order);
    double transaction_amount =
        order->product->price_per_unit * quantity_delivered;
    pooled_input_value -= transaction_amount;
    plan->firm->receive_payment(plan, transaction_amount);
    log_shipment_received(order->product, quantity_delivered);
    log_inventory_level(order->product, input_inventory[order->product]);
}

void Firm::receive_payment(Plan * plan, double transaction_amount) {
    plan->debt += transaction_amount;
    pooled_input_value += transaction_amount;
}

bool Firm::remove_input_caused_by_firm(
        Product * product,
        double quantity,
        Firm * firm,
        std::unordered_map<Product *, double>& deducted_inputs
    ) {
    if (remove_input_caused_by_firm(product, quantity, firm)) {
        deducted_inputs[product] += quantity;
        return true;
    }
    return false;
}

bool Firm::remove_input_caused_by_firm(
        Product * product,
        double quantity,
        Firm * firm
        ) {
    if (!input_inventory.count(product) || input_inventory[product] < quantity) {
        return false;
    }
    input_inventory[product] -= quantity;
    add_demand_signal_caused_by_firm(product, quantity, firm);
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

double Firm::get_pooled_input_value() {
    return pooled_input_value;
}

std::vector<Person *> Firm::propose_transfer(int workers_wanted) {
    double firm_busyness = get_busyness();
    double societal_busyness = Society::get_instance()->get_busyness();
    double adjusted_societal_busyness =
        societal_busyness - TRANSFER_BUSYNESS_THRESHOLD;
    if (adjusted_societal_busyness <= 0 ||
            firm_busyness >= adjusted_societal_busyness) {
        log_busyness(firm_busyness, societal_busyness, 0);
        return {};
    }
    int available_workers_for_transfer = std::max(
        static_cast<int>(
            workers.size() * (1.0 - firm_busyness / adjusted_societal_busyness)
        ),
        0
    ); 
    int max_workers_to_transfer = std::min(available_workers_for_transfer, workers_wanted);
    log_busyness(firm_busyness, societal_busyness, max_workers_to_transfer);
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
    double order_rate = 0.0;
    Producer * chosen_producer = nullptr;
    Order * chosen_return_order = nullptr;
    std::vector<Producer *>& suppliers =
        Society::get_instance()->get_suppliers(order->product);
    for (Producer * producer : suppliers) {
        Order * return_order = producer->draft_plan_and_return_order(order);
        if (return_order->status == Order::kOrderRejected) {
            continue;
        }
        double return_order_rate =
            static_cast<double>(return_order->quantity) /
            return_order->requested_turnaround_time;

        if (return_order_rate > order_rate) {
            if (chosen_producer) {
                chosen_producer->drop_order(this);
            }
            order_rate = return_order_rate;
            chosen_producer = producer;
            chosen_return_order = return_order;
        } else {
            producer->drop_order(this);
        }
    }
    if (chosen_producer) {
        chosen_producer->pursue_order(this);
        product_to_outbound_orders[order->product].insert(chosen_return_order);
        log_reorder(order->product, chosen_return_order->quantity);
        log_accepted_order(order, chosen_return_order);
    }
    return chosen_producer;
}

double Firm::get_reorder_threshold(Product * product) {
    return get_demand(product) * FIRM_STOCKPILE_DURATION;
}

double Firm::get_resupply_deficit(Product * product) {
    double inventory = get_pending_inventory(product);
    double reorder_threshold = get_reorder_threshold(product);

    double resupply_rate = 0.0;
    for (Order * order : product_to_outbound_orders[product]) {
        resupply_rate += order->quantity / order->predicted_turnaround_time;
    }
    double inventory_adjusted_demand = get_demand(product)
        * std::pow(reorder_threshold / inventory, FIRM_REORDER_URGENCY);
    double raw_resupply_deficit = inventory_adjusted_demand - resupply_rate;
    double resupply_deficit = 
        raw_resupply_deficit <= 0 ? 0 : raw_resupply_deficit;
    return resupply_deficit;
}

void Firm::check_and_reorder_input(Product * product) {
    double resupply_deficit = get_resupply_deficit(product);
    if (resupply_deficit <= 0) {
        return;
    }
    double reorder_threshold = get_reorder_threshold(product);
    log_demand(product, reorder_threshold);
    double inventory = get_pending_inventory(product);
    if (inventory >= reorder_threshold || !reorder_threshold) {
        return;
    }
    Logger::log(
        get_client_type(),
        id,
        "resupply_rate_info",
        LogPair("product_id", product->id),
        LogPair("resupply_deficit", resupply_deficit)
    );

    int lead_time = FIRM_STOCKPILE_DURATION * FIRM_REORDER_DEADLINE_PROP;
    int order_quantity = std::ceil(lead_time * resupply_deficit);
    Order * order = new Order(
            product,
            order_quantity,
            this,
            lead_time
            );
    send_order(order);
}

void Firm::return_inputs_to_inventory(
        const std::unordered_map<Product *, double> deducted_inputs,
        Firm * firm
    ) {
    for (const std::pair<Product * const, double>& deduction :
            deducted_inputs) {
        Product * input = deduction.first;
        double quantity = deduction.second;
        input_inventory[input] += quantity;
        add_demand_signal_caused_by_firm(input, -quantity, firm);
        log_inventory_level(input, input_inventory[input]);
    }
}

void Firm::rollback_plan_inputs(Plan * plan, Firm * firm) {
    return_inputs_to_inventory(plan->inventory, firm);
    plan->inventory.clear();
    plan->outlays.clear();
    plan->order->status = Order::OrderStatus::kOrderRequested;
}

void Firm::start_plan(Plan * plan) {
    plans_in_progress.insert(plan);
    plan->order->status = Order::kOrderInProgress;
    Product * product = plan->order->product;
    for (std::pair<Good * const, double>& input : product->inputs_per_unit) {
        double amount_needed = input.second * plan->order->quantity;
        if (!remove_input_caused_by_firm(
                    input.first,
                    amount_needed,
                    plan->order->customer,
                    plan->inventory
                )) {
            rollback_plan_inputs(plan, plan->order->customer);
            return;
        }
    }
    double expected_plan_duration = plan->labor_budget / plan->workers.size();
    for (Machine * machine : product->machines_needed) {
        double expected_machine_use = expected_plan_duration / machine->lifetime;
        if (!remove_input_caused_by_firm(
                    machine,
                    expected_machine_use,
                    plan->order->customer,
                    plan->inventory
                    )) {
            rollback_plan_inputs(plan, plan->order->customer);
            return;
        }
    }
    for (Person * worker : plan->workers) {
        move_worker_off_standby(worker);
    }
    plan->outlays = plan->inventory;
    pooled_input_value +=
        plan->raw_materials_budget + plan->machinery_budget;
}

void Firm::move_plan_forward_one_step(Plan * plan) {
    double expected_quantity_produced =
        calculate_quantity_produced_from_worker_suitability(plan);
    double quantity_produced =
        std::min(expected_quantity_produced, plan->quantity_remaining);
    if (quantity_produced <= 0.0) {
        end_plan(plan);
        return;
    }
    Product * product = plan->order->product;
    for (std::pair<Good *, double> input : product->inputs_per_unit) {
        plan->needed_this_step[input.first] = input.second * quantity_produced;
    }
    double portion_of_hour_worked =
        quantity_produced / expected_quantity_produced;
    for (Machine * machine : product->machines_needed) {
        plan->needed_this_step[machine] =
            portion_of_hour_worked / machine->lifetime;
    }
    std::unordered_map<Product*, double> deducted_inputs;
    for (std::pair<Product *, double> requirement : plan->needed_this_step) {
        double have_on_hand = plan->inventory[requirement.first];
        if (have_on_hand < requirement.second) {
           double deficit = requirement.second - have_on_hand;
            if (!remove_input_caused_by_firm(
                        requirement.first,
                        deficit,
                        plan->order->customer,
                        deducted_inputs
                        )) {
                return_inputs_to_inventory(deducted_inputs, plan->order->customer);

                return;
            }
        }
    }
    for (std::pair<Product *, double> input : deducted_inputs) {
        plan->inventory[input.first] += input.second;
        plan->outlays[input.first] += input.second;
    }
    for (std::pair<Product *, double> requirement : plan->needed_this_step) {
        plan->inventory[requirement.first] -= requirement.second;
    }
    for (Person * worker : plan->workers) {
    	worker->register_hours_worked(portion_of_hour_worked);
    }
    plan->labor_hours_used += portion_of_hour_worked * plan->workers.size();
    plan->quantity_remaining -= quantity_produced;
}

void Firm::end_plan(Plan * plan) {
    log_ended_plan(plan);
    plan->order->status = Order::kOrderFinished;
    plan->order->customer->receive_shipment(plan);
    for (std::pair<Product *, double> input : plan->inventory) {
        plan->outlays[input.first] -= input.second;
    }
    return_inputs_to_inventory(
        plan->inventory,
        plan->order->customer
    );
    recorded_living_labor_per_unit[plan->order->product] = 
        plan->labor_hours_used /
        (plan->order->quantity - plan->quantity_remaining); 
    PriceController::get_instance()->update_price(plan);
    for (Person * worker : plan->workers) {
        standby_workers.insert(worker);
    }
}

void Firm::move_plans_forward_one_step() {
    std::unordered_set<Plan *> plans_still_in_progress;
    for (Plan * plan : plans_in_progress) {
        if (plan->order->status == Order::kOrderInProgress) {
            if (is_within_work_schedule(plan)) {
                move_plan_forward_one_step(plan);
            }
            if (plan->order->status == Order::kOrderInProgress &&
                    plan->quantity_remaining <= 0.0) {
                end_plan(plan);
            } else if (plan->order->status == Order::kOrderRequested) {
                start_plan(plan);
            }
        }
    }
    for (Plan * plan : plans_in_progress) {
        if (plan->order->status != Order::kOrderFinished) {
            plans_still_in_progress.insert(plan);
        }
    }
    plans_in_progress = plans_still_in_progress;
}

double Firm::calculate_quantity_produced_from_worker_suitability(Plan * plan) {
    double total_worker_suitability = 0.0;
    for (Person * worker : plan->workers) {
        total_worker_suitability +=
            worker->suitability(plan->order->product->required_abilities);
    }
    if (total_worker_suitability <= 0.0) {
        return 0.0;
    }
    return total_worker_suitability / 
        Society::get_instance()->get_underlying_living_labor_per_unit(plan->order->product);
}

bool Firm::is_within_work_schedule(Plan * plan) const {
    int time = Sim::get_current_time_step();
    return time % DAY < Society::get_instance()->get_current_work_hours_daily() &&
        time / DAY % 7 < Society::get_instance()->get_current_work_days_weekly();
}

double Firm::get_pending_inventory(Product * product) {
    double pending_inventory = get_inventory_level(product);
    for (Order * order : product_to_outbound_orders[product]) {
        pending_inventory += order->quantity;
    }
    return pending_inventory;
}

static double get_work_week_proportion() {
    return static_cast<double>(Society::get_instance()->get_current_work_hours_daily()) 
        * Society::get_instance()->get_current_work_days_weekly()
        / WEEK;
}

int Firm::predict_workers_needed(const Order * order) {
    double work_time = 
        order->requested_turnaround_time
        * get_work_week_proportion();
    return std::ceil(
            order->quantity *
            recorded_living_labor_per_unit[order->product] /
            work_time
            );
}

std::vector<Person *> Firm::get_available_workers(const Order * order) {
    std::vector<Person *> sorted_standby_workers(standby_workers.begin(),
            standby_workers.end());
    std::sort(sorted_standby_workers.begin(), sorted_standby_workers.end(), 
            [&](Person * a, Person * b) {
            return a->get_busyness() < b->get_busyness();
            });

    std::vector<Person *> available_workers;
    int workers_left = predict_workers_needed(order);
    for (Person * unemployed_person : Society::get_instance()->get_unemployed_people()) {
        if (workers_left <= 0) {
            break;
        } 
        available_workers.push_back(unemployed_person);
        workers_left--;
    }
    for (Person * worker : sorted_standby_workers) {
        if (workers_left <= 0) { 
            break;
        }
        available_workers.push_back(worker);
        workers_left--;
    }
    for (Firm * firm : Society::get_instance()->get_firms()) {
        if (workers_left <= 0) {
            break;
        } 
        if (firm == this) continue;
        std::vector<Person *> transfers = firm->propose_transfer(workers_left);
        log_transfer_request();
        for (Person * transfer : transfers) {
            available_workers.push_back(transfer);
        }
        workers_left -= transfers.size();
    }
    return available_workers;
}

void Firm::adjust_quantity_for_deadline(Plan * plan) {
    double living_labor_per_timestep = plan->workers.size() * get_work_week_proportion();
    int maximum_quantity_for_deadline = 
        plan->order->requested_turnaround_time *
        living_labor_per_timestep / 
        recorded_living_labor_per_unit[plan->order->product];
    plan->order->quantity = 
        std::min(plan->order->quantity, maximum_quantity_for_deadline);
}

double Firm::predict_turnaround_time(Plan * plan) {
    if (plan->workers.empty()) {
        return std::numeric_limits<double>::infinity();
    }
    double labor_hours_per_timestep = 
        plan->workers.size()
        * get_work_week_proportion();
    return plan->order->quantity *
           recorded_living_labor_per_unit[plan->order->product] /
           labor_hours_per_timestep;
}

double Firm::predict_labor_hours(Order * order, std::vector<Person *>& workers) {
    if (workers.empty()) {
        throw std::runtime_error("Cannot predict labor hours with 0 workers: "
                "Product " + std::to_string(order->product->id));
    }
    return order->quantity *
        recorded_living_labor_per_unit[order->product];
}

double Firm::calculate_raw_material_cost_for_order(Order * order) {
    double raw_material_cost = 0;
    for (std::pair<Good * const, double>& input : order->product->inputs_per_unit) {
        raw_material_cost += input.second * order->quantity *
            input.first->price_per_unit;
    }
    return raw_material_cost;
}

void Firm::initialize_plan_budget(Plan * draft_plan) {
    draft_plan->labor_budget = 
        predict_labor_hours(draft_plan->order, draft_plan->workers); 
    draft_plan->machinery_budget = calculate_machinery_cost_for_plan(draft_plan);
    draft_plan->raw_materials_budget = calculate_raw_material_cost_for_order(draft_plan->order);
    draft_plan->quantity_remaining = draft_plan->order->quantity;
    draft_plan->debt = -(
            draft_plan->machinery_budget +
            draft_plan->raw_materials_budget +
            draft_plan->labor_budget
            );
}

double Firm::calculate_machinery_cost_for_plan(Plan * draft_plan) {
    double machinery_cost_per_hour = 0.0;
    for (Machine * machine : draft_plan->order->product->machines_needed) {
        machinery_cost_per_hour += machine->price_per_unit / machine->lifetime;
    }
    if (draft_plan->workers.empty()) {
        throw std::runtime_error("Cannot calculate machinery cost with 0 workers: "
               "Product " + std::to_string(draft_plan->order->product->id));
    }
    return machinery_cost_per_hour *
        draft_plan->labor_budget / draft_plan->workers.size();
}

void Firm::assign_plan_dependent_fields(Plan * draft_plan) {
    draft_plan->order->predicted_turnaround_time =
        predict_turnaround_time(draft_plan);
    initialize_plan_budget(draft_plan);
}

Plan * Firm::draft_plan_for_order(Order * order) {
    Plan * draft_plan = new Plan;
    draft_plan->order = order;
    draft_plan->firm = this;
    draft_plan->local_work_hours_daily = 
        Society::get_instance()->get_current_work_hours_daily();
    draft_plan->workers = get_available_workers(draft_plan->order);
    adjust_quantity_for_deadline(draft_plan);
    if (order->quantity <= 0) {
        log_reorder_failure(order->product, "not_enough_workers_available");
        delete draft_plan;
        return nullptr;
    }
    if (draft_plan->workers.empty()) {
        if (predict_workers_needed(draft_plan->order) > 0) {
            log_reorder_failure(order->product, "no_workers_available");
        } else {
            log_reorder_failure(order->product, "bogus_order");
        }
        delete draft_plan;
        return nullptr;
    }
    assign_plan_dependent_fields(draft_plan);
    return draft_plan;
}

void Firm::add_demand_signal_caused_by_firm(Product * product, double quantity, Firm * firm) {
    if (firm->get_client_type() == Logger::PRODUCER) {
        reproduction_demands[product] += quantity / REPRODUCTION_DEMAND_AVERAGING_WINDOW;
    } else if (firm->get_client_type() == Logger::DISTRIBUTOR) {
        consumer_demands[product] += quantity / CONSUMER_DEMAND_AVERAGING_WINDOW;
    }
}

double Firm::get_demand(Product * product) {
    return reproduction_demands[product] + consumer_demands[product];
}

void Firm::update_demands() {
    for (std::pair<Product * const, double>& demand : reproduction_demands) {
        double decay = demand.second / REPRODUCTION_DEMAND_AVERAGING_WINDOW;
        demand.second -= decay;
    }
    for (std::pair<Product * const, double>& demand : consumer_demands) {
        double decay = demand.second / CONSUMER_DEMAND_AVERAGING_WINDOW;
        demand.second -= decay;
    }
}

void Firm::move_worker_off_standby(Person * worker) {
    if (worker->get_firm() == nullptr) {
        Society::get_instance()->get_unemployed_people().erase(worker);
        log_initial_employment(worker->get_id(), id);
    } else if (worker->get_firm() == this) {
        standby_workers.erase(worker);
    } else {
        int old_employer = worker->get_firm()->get_id();
        worker->get_firm()->finalize_transfer(worker);
        log_employment_transfer(worker->get_id(), old_employer, this->get_id());
    }
    worker->set_firm(this);
    workers.insert(worker);
}

void Firm::log_plans() {
    for (Plan * plan : plans_in_progress) {
        Logger::log(
                get_client_type(),
                id,
                "plan_quantity_remaining",
                LogPair("product_id", plan->order->product->id),
                LogPair("quantity_remaining", plan->quantity_remaining)
                );
    }
}

void Firm::log_pursued_plan(const Plan * draft_plan) {
    Order * order = draft_plan->order;
    Logger::log(
            get_client_type(),
            id,
            "pursued_plan",
            LogPair("customer_id", order->customer->get_id()),
            LogPair("product_id", order->product->id),
            LogPair("quantity", order->quantity),
            LogPair("num_workers", draft_plan->workers.size()),
            LogPair("lead_time", order->predicted_turnaround_time)
            );
}

void Firm::log_ended_plan(const Plan * plan) {
    Order * order = plan->order;
    Logger::log(
            get_client_type(),
            id,
            "ended_plan",
            LogPair("product_id", order->product->id),
            LogPair("quantity", order->quantity)
            );
}

void Firm::log_shipment_received(const Product * product, const double quantity) {
    log_product_quantity("shipment_received", product, quantity);
}

void Firm::log_inventory_level(const Product * product, const double quantity) {
    log_product_quantity("inventory_level", product, quantity);
}

void Firm::log_inventory_reduction(const Product * product, const double quantity) {
    log_product_quantity("inventory_reduction", product, quantity);
}

void Firm::log_initial_employment(
        const unsigned int worker_id,
        const unsigned int workplace_id
        ) {
    Logger::log(
        get_client_type(),
        id,
        "newly_employed",
        LogPair("worker_id", worker_id)
    );
}

void Firm::log_employment_transfer(
    const unsigned int worker_id,
    const unsigned int old_workplace_id,
    const unsigned int new_workplace_id
) {
    Logger::log(
        get_client_type(),
        id,
        "transfer",
        LogPair("worker_id", worker_id),
        LogPair("old_workplace_id", old_workplace_id),
        LogPair("new_workplace_id", new_workplace_id)
    );
}

void Firm::log_busyness(
    double firm_busyness,
    double societal_busyness,
    int max_workers_for_transfer
) {
    Logger::log(
        get_client_type(),
        id,
        "busyness",
        LogPair("firm_busyness", firm_busyness),
        LogPair("societal_busyness", societal_busyness),
        LogPair("max_workers_for_transfer", max_workers_for_transfer)
    );
}

void Firm::log_reorder(const Product * product, const int quantity) {
    log_product_quantity("reorder", product, quantity);
}

void Firm::log_reorder_failure(const Product * product, const std::string reason) {
    // Logger::log(
    //     get_client_type(),
    //     id,
    //     "reorder_failure",
    //     LogPair("product_id", product->id),
    //     LogPairS("reason", reason)
    // );
}

void Firm::log_product_quantity(
        const char * const label,
        const Product * product,
        const double quantity
        ) {
    Logger::log(
            get_client_type(),
            id,
            label,
            LogPair("product_id", product->id),
            LogPair("amount", quantity)
            );
}

void Firm::log_accepted_order(
        const Order * original_order,
        const Order * chosen_return_order
        ) {
    Logger::log(
            get_client_type(),
            id,
            "accepted_order",
            LogPair("product_id", original_order->product->id),
            LogPair("quantity", original_order->quantity),
            LogPair(
                "offered_turnaround_time",
                chosen_return_order->requested_turnaround_time
                ),
            LogPair("offered_quantity", chosen_return_order->quantity)
            );
}

void Firm::log_demand(const Product * product, double demand) {
    Logger::log(
            get_client_type(),
            id,
            "current_demand",
            LogPair("product_id", product->id),
            LogPair("demand", demand)
            );
}

void Firm::log_transfer_request() {
    Logger::log(get_client_type(), id, "transfer_request");
}

void Firm::log_catalog() {
    std::ostringstream oss;
    bool first = true;
    for (Product * product : catalog) {
        if (first) {
            oss << product->id;
            first = false;
        } else {
            oss << "," << product->id;
        }
    }
    Logger::log(get_client_type(), id, "catalog", LogPairS("product_ids", oss.str()));
}

void Firm::log_catalog_addition(Product * product) {
    Logger::log(get_client_type(), id, "catalog_addition", LogPair("product_id", product->id));
}
