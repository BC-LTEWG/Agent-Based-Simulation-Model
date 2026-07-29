#include <algorithm>
#include <iostream>
#include <unordered_map>
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
    double gross_hourly_demand_per_capita =
        Society::get_instance()->get_gross_hourly_demand_per_capita()[product];
    double gross_hourly_demand = gross_hourly_demand_per_capita * Sim::get_num_people();
    double gross_demand_per_producer = 
        gross_hourly_demand /
        Society::get_instance()->get_number_of_producers_for_product()[product];

    double starting_num_firms =
        Sim::get_num_producers() +
        Sim::get_num_distributors();

    average_team_sizes[product] = std::max<double>(
            Sim::get_num_people() / starting_num_firms,
            1.0
        );
    for (Machine * machine : product->machines_needed) {
        double machine_use_per_unit =
            product->living_labor_per_unit / 
            (machine->lifetime * average_team_sizes[product]);
        producer_demands[machine] +=
            machine_use_per_unit * gross_demand_per_producer;
    }
    for (std::pair<Good * const, double>& input :
            product->inputs_per_unit) {
        producer_demands[input.first] +=
            input.second * gross_demand_per_producer;
    }


    log_catalog_addition(product);
}

void Producer::initialize_inventory() {
     std::normal_distribution<double> input_noise_dist(
            1.0, DEMAND_PREDICTION_VARIANCE);

    for (std::pair<Product * const, double>& demand : producer_demands) {
        double noise = std::max(
            input_noise_dist(Sim::get_random_generator()),
            0.01
        );
        double input_amount_added = get_reorder_threshold(demand.first) * noise;
        input_inventory[demand.first] = input_amount_added;
        log_inventory_level(demand.first, input_amount_added);
    }
}

bool Producer::can_produce(Product * product) {
    return catalog.count(product);
}

double Producer::get_max_order_quantity(const Order * order) {
    double max_order_quantity = std::numeric_limits<double>::infinity();
    for (std::pair<Good * const, double>& input : order->product->inputs_per_unit) {
        if (input.second <= 0.0) {
            continue;
        }
        double input_max_order_quantity = 
                get_inventory_level(input.first) / input.second;
        max_order_quantity = 
            std::min(max_order_quantity, input_max_order_quantity);
    }
    std::vector<Person *> available_workers = get_available_workers(order);
    for (Machine * machine : order->product->machines_needed) {
        double machine_max_order_quantity =
            (input_inventory[machine] * machine->lifetime) *
            available_workers.size() /
            recorded_living_labor_per_unit[order->product];
        max_order_quantity =
            std::min(max_order_quantity, machine_max_order_quantity);
    }
    return max_order_quantity;
}

// double Producer::get_estimated_max_order_quantity(Product * product) {
//     double max_order_quantity = std::numeric_limits<double>::infinity();
//     for (std::pair<Good * const, double>& input : product->inputs_per_unit) {
//         if (input.second <= 0.0) {
//             continue;
//         }
//         double input_max_order_quantity = 
//                 get_inventory_level(input.first) / input.second;
//         max_order_quantity = 
//             std::min(max_order_quantity, input_max_order_quantity);
//     }
//     for (Machine * machine : product->machines_needed) {
//         double machine_max_order_quantity =
//             (input_inventory[machine] * machine->lifetime) *
//             average_team_sizes[product] /
//             recorded_living_labor_per_unit[product];
//         max_order_quantity =
//             std::min(max_order_quantity, machine_max_order_quantity);
//     }
//     return max_order_quantity;
// }

// double Producer::get_exact_max_order_quantity(Product * product, const Plan * draft_plan) {
//     double max_order_quantity = std::numeric_limits<double>::infinity();
//     for (std::pair<Good * const, double>& input : product->inputs_per_unit) {
//         if (input.second <= 0.0) {
//             continue;
//         }
//         double input_max_order_quantity = 
//                 get_inventory_level(input.first) / input.second;
//         max_order_quantity = 
//             std::min(max_order_quantity, input_max_order_quantity);
//     }
//     for (Machine * machine : product->machines_needed) {
//         double machine_max_order_quantity =
//             (input_inventory[machine] * machine->lifetime) *
//             draft_plan->workers.size() /
//             recorded_living_labor_per_unit[product];
//         max_order_quantity =
//             std::min(max_order_quantity, machine_max_order_quantity);
//     }
//     return max_order_quantity;
// }

// void Producer::trim_worker_assignment(Plan * draft_plan) {
//     int required_workers = predict_workers_needed(draft_plan);
    
//     while (static_cast<int>(draft_plan->workers.size()) > required_workers) {
//         draft_plan->workers.pop_back();
//     }
// }

// void Producer::adjust_workers_for_quantity(Plan * draft_plan) {
//     while (true) {
//         int old_quantity = draft_plan->order->quantity;
//         std::size_t old_worker_count = draft_plan->workers.size();

//         trim_worker_assignment(draft_plan);

//         if (draft_plan->workers.empty()) {
//             draft_plan->order->quantity = 0;
//             return;
//         }

//         double exact_max_quantity = 
//             get_exact_max_order_quantity(
//                 draft_plan->order->product,
//                 draft_plan
//             );

//         if (exact_max_quantity < draft_plan->order->quantity) {
//             draft_plan->order->quantity = 
//                 static_cast<int>(exact_max_quantity);
//         }

//         if (draft_plan->order->quantity <= 0) {
//             return;
//         }

//         if (draft_plan->order->quantity == old_quantity && 
//                 draft_plan->workers.size() == old_worker_count) {
//             return;
//         }
//     }
// }

Order * Producer::draft_plan_and_return_order(const Order * order) {
    Order * return_order = new Order(
        order->product,
        order->quantity,
        order->customer,
        order->requested_turnaround_time
    );
    return_order->quantity = std::min(
        return_order->quantity,
        static_cast<int>(get_max_order_quantity(order))
    );
    Plan * draft_plan = draft_plan_for_order(return_order);
    if (!draft_plan) {
        delete draft_plan;
        return_order->status = Order::kOrderRejected;
        return return_order;
    }

    assign_plan_dependent_fields(draft_plan);
    customer_to_draft_plan[order->customer] = draft_plan;
    log_draft_plan(draft_plan);

    return return_order;

    // double estimated_max_order_quantity = 
    //     get_estimated_max_order_quantity(order->product);

    // if (estimated_max_order_quantity < return_order->quantity) {
    //     return_order->quantity = static_cast<int>(estimated_max_order_quantity);
    // }
    // if (return_order->quantity <= 0) {
    //         return_order->status = Order::kOrderRejected;
    //         log_reorder_failure(order->product, "insufficient_resources");
    //         return return_order;
    // }

    // Plan * draft_plan = draft_plan_for_order(return_order);

    // if (!draft_plan) {
    //     return_order->status = Order::kOrderRejected;
    //     return return_order;
    // }

    // adjust_workers_for_quantity(draft_plan);

    // if (draft_plan->order->quantity <= 0 || draft_plan->workers.empty()) {
    //     delete draft_plan;
    //     return_order->status = Order::kOrderRejected;
    //     log_reorder_failure(order->product, "insufficient_resources");
    //     return return_order;
    // }

    // assign_plan_dependent_fields(draft_plan);
    // customer_to_draft_plan[order->customer] = draft_plan;
    // log_draft_plan(draft_plan);

    // return return_order;
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
