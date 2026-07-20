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
    for (std::pair<Product * const, double>& demand : demands) {
        check_and_reorder_input(demand.first);
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
    input_inventory[order->product] += order->quantity;
    product_to_outbound_orders[order->product].erase(order);
    double transaction_amount = order->product->price_per_unit * order->quantity;
    pooled_input_value -= transaction_amount;
    plan->firm->receive_payment(plan, transaction_amount);
    log_shipment_received(order->product, order->quantity);
    log_inventory_level(order->product, input_inventory[order->product]);
}

void Firm::receive_payment(Plan * plan, double transaction_amount) {
    plan->debt += transaction_amount;
    pooled_input_value += transaction_amount;
}

bool Firm::remove_input_from_inventory(
        Product * product,
        double quantity,
        std::vector<std::pair<Product *, double>>& deducted_inputs
    ) {
    if (remove_input_from_inventory(product, quantity)) {
       deducted_inputs.push_back(
            std::make_pair(product, quantity)
        );
        return true;
    }
    return false;
}


bool Firm::remove_input_from_inventory(Product * product, double quantity) {
    if (!input_inventory.count(product) || input_inventory[product] < quantity) {
        return false;
    }
    input_inventory[product] -= quantity;
    add_demand_signal(product, quantity);
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
    int quantity = 0;
    Producer * chosen_producer = nullptr;
    Order * chosen_return_order = nullptr;
    std::vector<Producer *>& suppliers =
        Society::get_instance()->get_suppliers(order->product);
    for (Producer * producer : suppliers) {
        Order * return_order = producer->draft_plan_and_return_order(order);
        if (return_order->status == Order::kOrderRejected) {
            continue;
        }
        if (return_order->quantity > quantity) {
            if (chosen_producer) {
                chosen_producer->drop_order(this);
            }
            quantity = return_order->quantity;
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
        // log_accepted_order(order, chosen_return_order);
    }
    return chosen_producer;
}

double Firm::get_reorder_threshold(Product * product) {
    return demands[product] * FIRM_STOCKPILE_DURATION;
}

double Firm::get_needed_resupply_rate(Product * product) {
    double needed_resupply_rate = demands[product];
    for (Order * order : product_to_outbound_orders[product]) {
        double resupply_rate = order->quantity / order->predicted_turnaround_time;
        needed_resupply_rate -= resupply_rate;
    }
    return needed_resupply_rate <= 0 ? 0 : needed_resupply_rate;
}

void Firm::check_and_reorder_input(Product * product) {
    double resupply_rate = get_needed_resupply_rate(product);
    if (resupply_rate <= 0) {
        return;
    }
    double reorder_threshold = get_reorder_threshold(product);
    log_demand(product, reorder_threshold);
    double inventory = get_inventory_level(product);
    if (inventory >= reorder_threshold || !reorder_threshold) {
        return;
    }
    int order_quantity = std::ceil(reorder_threshold);
    Order * order = new Order(
            product,
            order_quantity,
            this,
            order_quantity / resupply_rate
            );
    if (!send_order(order)) {
        log_reorder_failure(product, order->quantity);
    }
}

void Firm::rollback_plan_inputs(
    Plan * plan,
    const std::vector<std::pair<Product *, double>>& deducted_inputs
) {
    for (
        const std::pair<Product *, double>& deduction :
        deducted_inputs
    ) {
        Product * input = deduction.first;
        double quantity = deduction.second;

        input_inventory[input] += quantity;
        log_inventory_level(input, input_inventory[input]);
    }

    plan->inventory.clear();
    plan->outlays.clear();
}

void Firm::start_plan(Plan * plan) {
    std::vector<std::pair<Product *, double>> deducted_inputs;

	for (std::pair<Good * const, double>& input :
            plan->order->product->inputs_per_unit) {
        double required_input = input.second * plan->order->quantity;
        if (!remove_input_from_inventory(input.first, required_input, deducted_inputs)) {
            rollback_plan_inputs(plan, deducted_inputs);
            return;
        }
	}

    for (Person * worker : plan->workers) {
        move_worker_off_standby(worker);
    }

    plan->outlays = plan->inventory;
    pooled_input_value += plan->raw_materials_budget;
    plans_in_progress.insert(plan);
    plan->order->status = Order::kOrderInProgress;
}

void Firm::move_plan_forward_one_step(Plan * plan) {
    double ideal_quantity_produced = calculate_quantity_produced_from_worker_suitability(plan);
    double quantity_produced = std::min(ideal_quantity_produced, plan->quantity_remaining);
    if (quantity_produced <= 0.0) {
        return;
    }

    double labor_hours_per_worker = quantity_produced / ideal_quantity_produced;
	for (Person * worker : plan->workers) {
		worker->register_hours_worked(labor_hours_per_worker);
	}
    plan->labor_hours_used += labor_hours_per_worker * plan->workers.size();
    plan->quantity_remaining -= quantity_produced;
}

void Firm::end_plan(Plan * plan) {
    log_ended_plan(plan);
    plan->order->status = Order::kOrderFinished;
    plan->order->customer->receive_shipment(plan);
    recorded_living_labor_per_unit[plan->order->product] = 
        plan->labor_hours_used /
        (plan->order->quantity - plan->quantity_remaining); 
    pooled_input_value += plan->order->quantity * plan->order->product->price_per_unit;
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
            if (plan->quantity_remaining <= 0.0) {
                end_plan(plan);
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
    return Sim::get_current_time_step() % DAY <
        plan->local_work_hours_daily &&
        Sim::get_current_time_step() / DAY % 7 <
        Society::get_instance()->get_current_work_days_weekly();
}

double Firm::get_max_order_quantity(Product * product) {
    double max_order_quantity = std::numeric_limits<double>::infinity();
    for (std::pair<Good * const, double>& input : product->inputs_per_unit) {
        if (input.second <= 0.0) {
            continue;
        }
        double input_max_order_quantity = 
                get_inventory_level(input.first) / input.second;
                
        max_order_quantity = 
            std::min(max_order_quantity, input_max_order_quantity);
    }
    return max_order_quantity;
}

std::vector<Person *> Firm::get_available_workers(int max_workers) {
    std::vector<Person *> available_workers;
    std::vector<Person *> sorted_standby_workers(standby_workers.begin(),
            standby_workers.end());
    std::sort(sorted_standby_workers.begin(), sorted_standby_workers.end(), 
            [&](Person * a, Person * b) {
            return a->get_busyness() < b->get_busyness();
            });

    for (Person * worker : sorted_standby_workers) {
        if (max_workers <= 0) { 
            return available_workers;
        }
        available_workers.push_back(worker);
        max_workers--;
    }
    for (Person * unemployed_person : Society::get_instance()->get_unemployed_people()) {
        if (max_workers <= 0) {
            return available_workers;
        } 
        available_workers.push_back(unemployed_person);
        max_workers--;
    }
    for (Firm * firm : Society::get_instance()->get_firms()) {
        if (max_workers <= 0) {
            return available_workers;
        } 
        log_transfer_request();
        if (firm == this) continue;
        std::vector<Person *> transfers = firm->propose_transfer(max_workers);
        for (Person * transfer : transfers) {
            available_workers.push_back(transfer);
        }
        max_workers -= transfers.size();
    }
    return available_workers;
}

int Firm::predict_workers_needed(Plan * plan) {
    return std::ceil(
            recorded_living_labor_per_unit[plan->order->product] *
            WEEK /
            Sim::get_work_days_weekly() / 
            plan->local_work_hours_daily /
            plan->order->requested_resupply_rate
            );
}

double Firm::predict_timely_quantity(Plan * plan) {
    return FIRM_STOCKPILE_DURATION * 
            FIRM_REORDER_MAX_PROP *
            plan->workers.size() * 
            plan->local_work_hours_daily *
            Sim::get_work_days_weekly() /
            WEEK /
            recorded_living_labor_per_unit[plan->order->product];
}

double Firm::predict_turnaround_time(Plan * plan) {
    if (plan->workers.empty()) {
        return std::numeric_limits<double>::infinity();
    }
    return plan->order->quantity *
           recorded_living_labor_per_unit[plan->order->product] *
           WEEK /
           Sim::get_work_days_weekly() / 
           plan->local_work_hours_daily /
           plan->workers.size();
}

double Firm::predict_labor_hours(Order * order, std::vector<Person *>& workers) {
    if (workers.empty()) {
        throw std::runtime_error("Cannot predict labor hours with 0 workers: " + order->product->product_name);
    }
    return order->quantity *
           recorded_living_labor_per_unit[order->product] / 
           workers.size();
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
    for (Machine * machine : machines) {
        machinery_cost_per_hour += machine->price_per_unit / machine->lifetime;
    }
    if (draft_plan->workers.empty()) {
        throw std::runtime_error("Cannot calculate machinery cost with 0 workers: " + draft_plan->order->product->product_name);
    }
    return machinery_cost_per_hour *
        (static_cast<double>(draft_plan->labor_budget) / draft_plan->workers.size());
}

void Firm::assign_plan_dependent_fields(Plan * draft_plan) {
    draft_plan->order->predicted_turnaround_time =
        predict_turnaround_time(draft_plan);
    initialize_plan_budget(draft_plan);
}

Plan * Firm::draft_plan_for_order(Order * order) {
    double max_order_quantity = std::min(static_cast<double>(order->quantity), get_max_order_quantity(order->product));
    if (max_order_quantity <= 0) {
        return nullptr;
    }

    Order * new_order = new Order(
            order->product,
            max_order_quantity,
            order->customer,
            order->requested_resupply_rate
            );
    Plan * draft_plan = new Plan;
    draft_plan->order = new_order;
    draft_plan->firm = this;
    draft_plan->local_work_hours_daily = 
        Society::get_instance()->get_current_work_hours_daily();

    int requested_workers = predict_workers_needed(draft_plan);
    std::vector<Person *> available_workers = get_available_workers(requested_workers);
    draft_plan->workers = available_workers;
    int timely_quantity = std::min(draft_plan->order->quantity, static_cast<int>(predict_timely_quantity(draft_plan)));
    if (timely_quantity <= 0) {
        delete draft_plan->order;
        delete draft_plan;
        return nullptr;
    }
    draft_plan->order->quantity = timely_quantity;
    assign_plan_dependent_fields(draft_plan);
    return draft_plan;
}

void Firm::add_demand_signal(Product * product, double quantity) {
    demands[product] += quantity / DEMAND_AVERAGING_WINDOW;
}

void Firm::update_demands() {
    for (std::pair<Product * const, double>& demand : demands) {
        double decay = demand.second / DEMAND_AVERAGING_WINDOW;
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
            LogPair("num_workers", draft_plan->workers.size())
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

void Firm::log_reorder_failure(const Product * product, const int quantity) {
    log_product_quantity("reorder_failure", product, quantity);
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
            LogPair("resupply_rate", original_order->requested_resupply_rate),
            LogPair("offered_quantity", chosen_return_order->quantity),
            LogPair("offered_resupply_rate",
                chosen_return_order->quantity / chosen_return_order->predicted_turnaround_time)
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

