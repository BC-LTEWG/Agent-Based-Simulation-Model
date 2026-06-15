#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <numeric>

#include "ConsumerGood.h"
#include "Distributor.h"
#include "Good.h"
#include "Logger.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Producer::Producer(
        const std::unordered_set<Product *>& initial_catalog
        ) :
    Firm()
{
    for (Product * product : initial_catalog) {
        add_to_catalog(product);
    }
}

Logger::Client Producer::get_client_type() {
    return Logger::PRODUCER;
}

void Producer::on_time_step() {
    Firm::on_time_step();
}

void Producer::add_to_catalog(Product * product) {
    catalog.insert(product);
    for (Machine * machine : product->machines_needed) {
        machines.insert(machine);
    }
    for (std::pair<Good * const, double>& input :
            product->inputs_per_unit) {
        double output_demand = Society::get_instance()->get_initial_production()[product];
        if (product->product_type == Product::TYPE_GOOD) {
            Good * good = static_cast<Good *>(product);
            output_demand = good->corresponding_consumer_good->mean_consumption_frequency;
        }
        input_inventory[input.first] += 
            input.second 
            * output_demand
            * Sim::get_num_people() 
            * Sim::get_num_goods() 
            / Sim::get_num_producers()
            * (FIRM_STOCKPILE_DURATION + DEMAND_ADAPTATION_DURATION * DEMAND_AVERAGING_WINDOW);
    }
    for (std::pair<Product * const, double>& stockpile : input_inventory) {
        log_inventory_level(stockpile.first, stockpile.second);
    }
    log_catalog_addition(product);
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

Order * Producer::draft_plan_and_return_order(const Order * order) {
    int return_order_quantity = std::min(order->quantity, get_max_order_quantity(order->product));
    Order * return_order = new Order(
            order->product,
            return_order_quantity,
            order->customer,
            std::max(1.0, order->requested_turnaround_time
            * return_order_quantity / order->quantity)
            );
	Plan * draft_plan = draft_plan_for_order(return_order);
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
	Plan * plan = customer_to_draft_plan[customer];
	if (!plan) {
        std::cerr << "Error: pursuing order from firm with no approved draft plan" << std::endl;
        return;
	}
	customer_to_draft_plan[customer] = nullptr;
    start_plan(plan);
    log_pursued_plan(plan);
    Society::get_instance()->log_total_employment();
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
