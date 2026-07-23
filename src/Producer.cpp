#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <numeric>

#include "ConsumerGood.h"
#include "Distributor.h"
#include "Good.h"
#include "Logger.h"
#include "Machine.h"
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
    double output_demand =
        Society::get_instance()->get_initial_production()[product];
    double demand_scale = output_demand * Sim::get_num_people() *
        Sim::get_num_goods() / Sim::get_num_producers();
    if (product->product_type == Product::ProductType::kTypeMachine) {
        demand_scale = 1.0;
    }
    double starting_num_firms =
        Sim::get_num_producers() + Sim::get_num_distributors();
    double average_team_size =
        std::max<double>(
                Sim::get_num_people() / starting_num_firms,
                1.0
                );
    double machine_use_per_unit =
        product->living_labor_per_unit / average_team_size;
    for (Machine * machine : product->machines_needed) {
        demands[machine] +=
            (machine_use_per_unit / machine->lifetime) * demand_scale;
    }
    for (std::pair<Good * const, double>& input :
            product->inputs_per_unit) {
        demands[input.first] += input.second * demand_scale;
    }
    static std::normal_distribution<double> demand_mult(
            1.0, DEMAND_PREDICTION_VARIANCE);
    for (std::pair<Product * const, double>& demand : demands) {
        double input_amount_added =
            demand.second * demand_mult(Sim::get_random_generator()) *
            FIRM_STOCKPILE_DURATION;
        if (demand.first->product_type == Product::ProductType::kTypeMachine) {
            input_amount_added = std::ceil(input_amount_added);
        }
        input_inventory[demand.first] = input_amount_added;
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
    for (Machine * machine : product->machines_needed) {
        double machine_max_order_quantity =
            (input_inventory[machine] * machine->lifetime) /
            recorded_living_labor_per_unit[machine];
        max_order_quantity =
            std::min(max_order_quantity, machine_max_order_quantity);
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
    if (order->product->product_type == Product::ProductType::kTypeMachine) {
        max_order_quantity = std::ceil(max_order_quantity);
    }
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
