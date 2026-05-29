#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <numeric>

#include "Distributor.h"
#include "Good.h"
#include "Logger.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Producer::Producer(
        Society * society,
        const std::unordered_set<Product *>& initial_catalog
        ) :
    Firm(society, initial_catalog) {
    std::unordered_set<Machine *> initial_machines;
    for (Product * product : catalog) {
        for (Machine * machine : product->machines_needed) {
            initial_machines.insert(machine);
        }
    }
    for (Machine * machine : initial_machines) {
        machines.push_back(machine);
    }
    for (Product * product : catalog) {
        for (std::pair<Good * const, double>& input :
                product->inputs_per_unit) {
            this->input_inventory[input.first] +=
                input.second * 
                society->get_initial_production()[product] * 
                (FIRM_STOCKPILE_DURATION + FIRM_DEMAND_WINDOW_MIN) *
                Sim::get_num_people() * Sim::get_num_products() / Sim::get_num_producers();
        }
    }
    for (std::pair<Product * const, double>& stockpile : input_inventory) {
        log_inventory_level(stockpile.first, stockpile.second);
    }
    log_catalog();
}

Logger::Client Producer::get_client_type() {
    return Logger::PRODUCER;
}

void Producer::on_time_step() {
    Firm::on_time_step();
}

bool Producer::can_produce(Product * product) {
    return catalog.count(product);
}

int Producer::get_max_order_quantity(Product * product) {
    int max_order_quantity = INT_MAX;
for (std::pair<Good * const, double>& input : product->inputs_per_unit) {
        int input_max_order_quantity = static_cast<int>(
                input_inventory[input.first] / input.second
                );
        max_order_quantity = std::min(max_order_quantity, input_max_order_quantity);
    }
    return max_order_quantity;
}

void Producer::add_order_input_demand_signals(const Order * order) {
    for (std::pair<Good * const, double>& input : order->product->inputs_per_unit) {
        add_demand_signal(input.first, input.second * order->quantity);
    }
}

Order * Producer::draft_plan_and_return_order(const Order * order) {
    int return_order_quantity = std::min(order->quantity, get_max_order_quantity(order->product));
    Order * return_order = new Order(
            order->product,
            return_order_quantity,
            order->customer,
            std::max(1.0, order->requested_turnaround_time
            * return_order_quantity / order->quantity)
            );
	Plan * draft_plan = draft_plan_with_required_abilities(return_order,
            order->product->required_abilities);
    if (!draft_plan) {
        return_order->status = Order::ORDER_REJECTED;
        return return_order;
    }
    return_order->requested_turnaround_time = draft_plan->predicted_turnaround_time;
	customer_to_draft_plan[order->customer] = draft_plan;
    log_draft_plan(draft_plan);
	return return_order;
}

void Producer::drop_order(Firm * customer) {
    log_dropped_order(customer_to_draft_plan[customer]->order);
    customer_to_draft_plan[customer] = nullptr;
}

void Producer::pursue_order(Firm * customer) {
	Plan * draft_plan = customer_to_draft_plan[customer];
	if (!draft_plan) {
        std::cerr << "Error: pursuing order from firm with no approved draft plan" << std::endl;
	}
    Order * order = draft_plan->order;
    add_order_input_demand_signals(order);
    for (Person * worker : draft_plan->workers) {
        move_worker_off_standby(worker);
    }

	// move draft_plan to plans_in_progress
	customer_to_draft_plan[customer] = nullptr;
	plans_in_progress.push_back(draft_plan);
    log_pursued_plan(draft_plan);
    society->log_total_employment();
    // accounting
    plans_in_progress.back()->prd +=
        order->product->price_per_unit * order->quantity;
}

void Producer::log_draft_plan(const Plan * draft_plan) {
    Order * order = draft_plan->order;
    Logger::log(
            Logger::PRODUCER,
            id,
            "draft_plan",
            LogPair("product_id", order->product->id),
            LogPair("quantity", order->quantity)
            );
}

void Producer::log_dropped_order(const Order * order) {
    Logger::log(
            Logger::PRODUCER,
            id,
            "dropped_order",
            LogPair("product_id", order->product->id),
            LogPair("quantity", order->quantity)
            );
}
