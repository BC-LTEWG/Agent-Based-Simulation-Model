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
#include "Plan.h"
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

void Distributor::add_to_catalog(Product * product) {
    catalog.insert(product);
    ConsumerGood * consumer_good = static_cast<ConsumerGood *>(product);
    Good * good = consumer_good->corresponding_good;
    consumer_demands[good] = 
        consumer_demands[consumer_good] = 
        consumer_good->mean_consumption_frequency 
        * Sim::get_num_people() 
        / Sim::get_num_distributors();
    static std::normal_distribution<double>
        demand_mult(1.0, DEMAND_PREDICTION_VARIANCE);
    consumer_demands[good] *= demand_mult(Sim::get_random_generator());
    consumer_demands[consumer_good] *= demand_mult(Sim::get_random_generator());
    input_inventory[good] = consumer_demands[good] * FIRM_STOCKPILE_DURATION;
    input_inventory[consumer_good] =
        consumer_demands[consumer_good] * FIRM_STOCKPILE_DURATION;
    log_inventory_level(good, input_inventory[good]);
    log_inventory_level(consumer_good, input_inventory[consumer_good]);
    log_catalog_addition(product);
}

int Distributor::try_sell_goods(
        ConsumerGood * consumer_good,
        int quantity, Person * person
        ) {
    int available = std::min(static_cast<int>(
                get_inventory_level(consumer_good)), quantity);
    if (!available) {
        return 0;
    }
    if (available < quantity) {
        log_shortfall(consumer_good->id, quantity - available);
    }
    double cost = available * consumer_good->price_per_unit;
    if (consumer_good->public_sector) {
        Society::get_instance()->charge_from_public_fund(cost);
    } else if (!person->charge(cost)) {
        return 0;
    }
    remove_input_from_inventory(consumer_good, available, this);
    PriceController::get_instance()->report_distribution(consumer_good, available);
    return available;
}

void Distributor::check_and_reorder_input(Product * product) {
    if (product->product_type != Product::ProductType::kTypeConsumerGood) {
        Firm::check_and_reorder_input(product);
        return;
    }
    double resupply_deficit = get_resupply_deficit(product);
    if (resupply_deficit <= 0) {
        return;
    }
    ConsumerGood * consumer_good = static_cast<ConsumerGood *>(product);
    Good * good = consumer_good->corresponding_good;

    double reorder_threshold = get_reorder_threshold(consumer_good);
    log_demand(consumer_good, reorder_threshold);

    double inventory = get_pending_inventory(consumer_good);
    if (inventory >= reorder_threshold || !reorder_threshold) {
        return;
    }
    
    int lead_time = 
        get_inventory_level(good) / resupply_deficit;

    lead_time = std::min(
        lead_time,
        static_cast<int>(
            FIRM_STOCKPILE_DURATION * 
            FIRM_REORDER_MAX_PROP
        )
    );
    // int order_quantity = lead_time * resupply_deficit;
    Order * order = new Order(
            consumer_good,
            lead_time * resupply_deficit,
            this,
            lead_time
            );
    if (Plan * plan = draft_plan_for_order(order)) {
        product_to_outbound_orders[consumer_good].insert(order);
        start_plan(plan);
        log_pursued_plan(plan);
    } else {
        log_reorder_failure(product, "no_workers_available");
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

