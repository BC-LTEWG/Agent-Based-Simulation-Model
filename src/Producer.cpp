#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <numeric>

#include "ConsumerGood.h"
#include "Distributor.h"
#include "Good.h"
#include "Logger.h"
#include "Order.h"
#include "Person.h"
#include "Plan.h"
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

void Producer::add_to_catalog(Product * product) {
    catalog.insert(product);
    for (Machine * machine : product->machines_needed) {
        machines.insert(machine);
    }
    for (std::pair<Good * const, double>& input :
            product->inputs_per_unit) {
        double output_demand = Society::get_instance()->get_initial_production()[product];
        double input_demand = 
            input.second 
            * output_demand
            * Sim::get_num_people() 
            / Society::get_instance()->get_product_production_count()[product];
        demands[input.first] += input_demand;
    }
    static std::normal_distribution<double> demand_mult(1.0, DEMAND_PREDICTION_VARIANCE);
    for (std::pair<Product * const, double>& demand : demands) {
        demand.second *= demand_mult(Sim::get_random_generator());
        input_inventory[demand.first] = demand.second * FIRM_STOCKPILE_DURATION;
    }
    for (std::pair<Product * const, double>& stockpile : input_inventory) {
        log_inventory_level(stockpile.first, stockpile.second);
    }
    log_catalog_addition(product);
}

bool Producer::can_produce(Product * product) {
    return catalog.count(product);
}

double Producer::get_max_order_quantity(Product * product) {
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

Order * Producer::draft_plan_and_return_order(const Order * order) {
    Order * return_order = new Order(
        order->product,
        order->quantity,
        order->customer,
        order->requested_turnaround_time
    );

    Plan * draft_plan = draft_plan_for_order(return_order);

    if (!draft_plan) {
        return_order->status = Order::kOrderRejected;
        return return_order;
    }

    double max_order_quantity = get_max_order_quantity(order->product);

    int feasible_quantity =
        static_cast<int>(
            std::min(static_cast<double>(order->quantity), max_order_quantity)
        );

    if (feasible_quantity <= 0) {
        delete draft_plan;
        return_order->status = Order::kOrderRejected;
        return return_order;
    }

    if (feasible_quantity != return_order->quantity) {
        delete draft_plan;
        return_order->quantity = feasible_quantity;
        return_order->requested_turnaround_time = std::max(
            1.0,
            order->requested_turnaround_time * 
            feasible_quantity / 
            order->quantity
        );

        draft_plan = draft_plan_for_order(return_order);

        if (!draft_plan) {
            return_order->status = Order::kOrderRejected;
            return return_order;
        }
    }

    return_order->requested_turnaround_time =
        draft_plan->predicted_turnaround_time;

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
