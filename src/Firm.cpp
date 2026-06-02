#include <algorithm>
#include <climits>
#include <iostream>
#include <numeric>
#include <sstream>

#include "Constants.h"
#include "Distributor.h"
#include "Firm.h"
#include "Logger.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Order::Order(
        Product * product,
        int quantity,
        Firm * customer,
        double requested_turnaround_time
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

double Firm::get_inventory_level(Product * product) {
    return input_inventory.count(product) ? input_inventory[product] : 0;
}

void Firm::add_supplier(Producer * producer) {
    suppliers.push_back(producer);
}

void Firm::receive_shipment(Plan * plan) {
    Order * order = plan->order;
    input_inventory[order->product] += order->quantity;
    product_to_outbound_orders[order->product].erase(order);
    double transaction_amount = order->product->price_per_unit * order->quantity;
    pooled_input_value_account -= transaction_amount;
    plan->firm->receive_payment(plan, transaction_amount);
    log_shipment_received(order->product, order->quantity);
    log_inventory_level(order->product, input_inventory[order->product]);
}

void Firm::receive_payment(Plan * plan, double transaction_amount) {
    plan->prd += transaction_amount;
}

bool Firm::remove_input_from_inventory(Product * product, double quantity) {
    if (!input_inventory.count(product) || input_inventory[product] < quantity) {
        return false;
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
    double firm_busyness = get_busyness();
    double societal_busyness = society->get_busyness();
    double adjusted_societal_busyness = societal_busyness - TRANSFER_BUSYNESS_THRESHOLD;
    if (adjusted_societal_busyness <= 0 || firm_busyness >= adjusted_societal_busyness) {
        log_busyness(firm_busyness, societal_busyness, 0);
        return {};
    }
    int max_workers_to_transfer = (int) (workers.size() * (1.0 - firm_busyness / adjusted_societal_busyness)); 
    max_workers_to_transfer = std::max(max_workers_to_transfer, workers_wanted);
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

    for (Producer * producer : suppliers) {
        if (!producer->can_produce(order->product)) {
            continue;
        }
        Order * return_order = producer->draft_plan_and_return_order(order);
        double return_order_rate =
            static_cast<double>(return_order->quantity) /
            return_order->requested_turnaround_time;
        if (return_order->status != Order::ORDER_REJECTED &&
                return_order_rate > order_rate) {
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

double Firm::get_pending_inventory_level(Product * product) {
    double pending_inventory = 0.0;
    if (input_inventory.count(product)) {
        pending_inventory = input_inventory[product];
    }
    if (product_to_outbound_orders.count(product)) {
        for (Order * order : product_to_outbound_orders[product]) {
            pending_inventory += order->quantity;
        }
    }
    return pending_inventory;
}

void Firm::check_and_reorder_inputs() {
    for (std::pair<Product *, double> stockpile : input_inventory) {
        check_and_reorder_input(stockpile.first);
    }
}

void Firm::check_and_reorder_input(Product * product) {
    double threshold = get_reorder_threshold(product);
    log_demand(product, threshold);
    int pending_inventory = get_pending_inventory_level(product);
    log_pending_inventory(product, pending_inventory);
    if (pending_inventory >= threshold || !threshold) return;
    Order * order = new Order(
            product,
            threshold * FIRM_REORDER_MAX_PROP,
            this,
            FIRM_STOCKPILE_DURATION * pending_inventory * FIRM_REORDER_MAX_PROP / threshold
            );
    if (!send_order(order)) {
        log_reorder_failure(product, order->quantity);
    }
}

int Firm::predict_workers_needed(Plan * plan) {
    return std::ceil(
            plan->order->quantity *
            plan->order->product->societal_living_labor_per_unit *
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
        log_transfer_request();
        if (producer == this) continue;
        std::vector<Person *> transfers = producer->propose_transfer(workers_left);
        for (Person * transfer : transfers) {
            draft_plan->workers.push_back(transfer);
        }
        workers_left -= transfers.size();
    }
}

double Firm::predict_turnaround_time(Plan * plan, std::vector<Person *>& workers) {
    double count = safe_handle_zero(static_cast<double>(workers.size()), "predict_turnaround_time workers", 0.0);
    if (count == 0) {
        return 0;
    }
    return plan->order->quantity *
           recorded_living_labor_per_unit[plan->order->product] *
           WEEK /
           Sim::get_work_days_weekly() / 
           plan->local_work_hours_daily /
           workers.size();
}

double Firm::predict_labor_hours(Order * order, std::vector<Person *>& workers) {

    double count = safe_handle_zero(static_cast<double>(workers.size()), "predict_labor_hours workers", 0.0);
    if (count == 0) {
        return 0;
    }
    return order->quantity *
           recorded_living_labor_per_unit[order->product] / 
           workers.size();
}

double Firm::calculate_raw_material_cost_for_order(Order * order) {
    double raw_material_cost = 0;
    for (std::pair<Product * const, double>& input : order->product->inputs_per_unit) {
        raw_material_cost += input.second * order->quantity *
            input.first->price_per_unit;
    }
    return raw_material_cost;
}

void Firm::initialize_plan_budget(
        Plan * draft_plan
        ) {
    double raw_material_cost = calculate_raw_material_cost_for_order(draft_plan->order);
    draft_plan->raw_materials =
        draft_plan->raw_materials_remaining = raw_material_cost;
    draft_plan->total_hours =
        draft_plan->total_hours_remaining =
        draft_plan->labor_hours + draft_plan->raw_materials;
    draft_plan->quantity_remaining = draft_plan->order->quantity;
    draft_plan->prd = -(draft_plan->total_hours);
}

double Firm::calculate_machinery_cost_for_plan(Plan * draft_plan) {
    double machinery_cost_per_hour = 0.0;
    for (Machine * machine : machines) {
        machinery_cost_per_hour += machine->price_per_unit / machine->lifetime;
    }
    double count = safe_handle_zero(static_cast<double>(draft_plan->workers.size()), "machinery_cost workers", 0.0);
    if (count == 0) {
        return 0;
    }
    return machinery_cost_per_hour *
        (static_cast<double>(draft_plan->labor_hours) / draft_plan->workers.size());
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
    draft_plan->machinery_cost = calculate_machinery_cost_for_plan(draft_plan);
}

Plan * Firm::draft_plan_with_required_abilities(
        Order * order,
        std::vector<Person::Ability>& required_abilities
        ) {
    Plan * draft_plan = new Plan{};
    draft_plan->order = order;
    draft_plan->firm = this;
    draft_plan->local_work_hours_daily = society->get_current_work_hours_daily();

    assign_workers(
        draft_plan,
        required_abilities
    );
    assign_plan_dependent_fields(
        draft_plan,
        required_abilities
    );
    return draft_plan;
}

void Firm::add_demand_signal(Product * product, double quantity) {
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
    if (demand_signals.count(product) && !demand_signals[product].empty()) {
        window_start = demand_signals[product].front().timestep;
    }
    int window_length = std::max(FIRM_DEMAND_WINDOW_MIN, 
        Sim::get_current_time_step() - window_start);
    if(window_length < 1) {
        throw std::runtime_error("Window length cannot be less than 1: " + product->product_name);
    }
    double total_d = 0.0;
    if (total_demands.count(product)) {
        total_d = total_demands[product];
    }
    return total_d / static_cast<double>(window_length);
}

void Firm::move_worker_off_standby(Person * worker) {
    if (worker->get_firm() == nullptr) {
        society->get_unemployed_people().erase(worker);
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

void Firm::log_pending_inventory(const Product * product, double pending_inventory) {
    Logger::log(
            get_client_type(),
            id,
            "pending_inventory",
            LogPair("product_id", product->id),
            LogPair("pending_inventory", pending_inventory)
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
    /*
    for (Product * product : catalog) {
        Logger::log(get_client_type(), id, "catalog", LogPair("product_id", product->id));
    }
    */
}

double Firm::safe_handle_zero(double value, const std::string& context, double fallback) {
    if (value <= 0) {
        std::cerr << "Firm " << id << " | " << context 
                  << " ran 0 or negative. Using safeguard: " << fallback << "\n";
        return fallback;
    }
    return value;
}