#include <algorithm>
#include <climits>
#include <iostream>
#include <string>

#include "Constants.h"
#include "ConsumerGood.h"
#include "Distributor.h"
#include "Good.h"
#include "Logger.h"
#include "Machine.h"
#include "Order.h"
#include "Person.h"
#include "PriceController.h"
#include "Producer.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Distributor::Distributor(
        const std::unordered_set<Product *>& initial_catalog
        ) :
    Firm()
{
    for (Product * product : initial_catalog) {
        add_to_catalog(product);
    }
}

Logger::Client Distributor::get_client_type() {
    return Logger::DISTRIBUTOR;
}

void Distributor::on_time_step() {
    Firm::on_time_step();

    for (Product * product : catalog) {
        ConsumerGood * consumer_good = static_cast<ConsumerGood *>(product);
        Good * good = consumer_good->corresponding_good;

        /*
        double good_per_consumer_good = 1.0;
        if (consumer_good->inputs_per_unit.count(good)) {
            good_per_consumer_good = consumer_good->inputs_per_unit[good];
        }

        demands[good] = std::max(
            demands[good],
            demands[consumer_good] * good_per_consumer_good
        );

        */
        Firm::check_and_reorder_input(good);
        check_and_reorder_input(consumer_good);
    }
}

void Distributor::add_to_catalog(Product * product) {
    catalog.insert(product);
    ConsumerGood * consumer_good = static_cast<ConsumerGood *>(product);
    Good * good = consumer_good->corresponding_good;
    demands[good] = 
        demands[consumer_good] = 
        consumer_good->mean_consumption_frequency 
        * Sim::get_num_people() 
        / Sim::get_num_distributors();
    static std::normal_distribution<double> demand_mult(1.0, DEMAND_PREDICTION_VARIANCE);
    demands[good] *= demand_mult(Sim::get_random_generator());
    demands[consumer_good] *= demand_mult(Sim::get_random_generator());
    input_inventory[good] = demands[good] * FIRM_STOCKPILE_DURATION;
    input_inventory[consumer_good] = demands[consumer_good] * FIRM_STOCKPILE_DURATION;
    log_inventory_level(good, input_inventory[good]);
    log_inventory_level(consumer_good, input_inventory[consumer_good]);
    log_catalog_addition(product);
}

int Distributor::try_sell_goods(ConsumerGood * consumer_good, int quantity, Person * person, bool record_failed_demand) {
    add_demand_signal(consumer_good, quantity);
    add_demand_signal(consumer_good->corresponding_good, quantity);
    int available = std::min(static_cast<int>(
                get_inventory_level(consumer_good)), quantity);
    if (!available) {
        if (record_failed_demand) {
            add_demand_signal(consumer_good, quantity);
        }
        return 0;
    }
    if (available < quantity) {
        log_shortfall(consumer_good->id, quantity - available);
    }
    double cost = available * consumer_good->price_per_unit;
    if (consumer_good->public_sector) {
        Society::get_instance()->charge_from_public_fund(cost);
    } else if (!person->charge(cost)) {
        if (record_failed_demand) {
            add_demand_signal(consumer_good, quantity);
        }
        return 0;
    }
    remove_input_from_inventory(consumer_good, available);
    PriceController::get_instance()->report_distribution(consumer_good, available);
    return available;
}

void Distributor::check_and_reorder_input(Product * product) {
    if (product->product_type != Product::kTypeConsumerGood) {
        Firm::check_and_reorder_input(product);
        return;
    }

    ConsumerGood * consumer_good = static_cast<ConsumerGood *>(product);
    Good * good = consumer_good->corresponding_good;

    double threshold = get_reorder_threshold(consumer_good);
    log_demand(consumer_good, threshold);

    double pending_consumer_goods_inventory = 
        get_pending_inventory_level(consumer_good);
    log_pending_inventory(consumer_good, pending_consumer_goods_inventory);
    if (pending_consumer_goods_inventory >= threshold || !threshold) {
        return;
    }

    int distribution_quantity = std::min(
            get_inventory_level(good),
            threshold * FIRM_REORDER_MAX_PROP
        );

    if (!distribution_quantity) {
        return;
    }
    double distribution_time = 
        static_cast<double>(distribution_quantity)
        / demands[consumer_good];
    Order * order = new Order(
            consumer_good,
            distribution_quantity,
            this,
            distribution_time
            );
    if (Plan * plan = draft_plan_for_order(order)) {
        product_to_outbound_orders[consumer_good].insert(order);
        start_plan(plan);
        log_pursued_plan(plan);
    } else {
        log_reorder_failure(product, order->quantity);
    }
}

void Distributor::log_shortfall(unsigned int product_id, int shortfall) {
    Logger::log(
            Logger::DISTRIBUTOR,
            id,
            "product_shortfall",
            LogPair("product_id", product_id),
            LogPair("shortfall", shortfall)
            );
}

