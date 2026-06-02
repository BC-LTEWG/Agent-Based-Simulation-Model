#include <algorithm>
#include <climits>
#include <iostream>
#include <string>

#include "ConsumerGood.h"
#include "Distributor.h"
#include "Good.h"
#include "Logger.h"
#include "Machine.h"
#include "Person.h"
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
    std::unordered_set<ConsumerGood *> consumer_goods_without_plans_copy =
        consumer_goods_without_plans;
    for (ConsumerGood * consumer_good : consumer_goods_without_plans_copy) {
        renew_distribution_plan(consumer_good);
    }
}

void Distributor::add_to_catalog(Product * product) {
    catalog.insert(product);
    ConsumerGood * consumer_good = static_cast<ConsumerGood *>(product);
    Good * good = consumer_good->corresponding_good;
    input_inventory[consumer_good] = 
        consumer_good->mean_consumption_frequency 
        * Sim::get_num_people() 
        / Sim::get_num_distributors()
        * (FIRM_STOCKPILE_DURATION + DEMAND_AVERAGING_WINDOW);
    consumer_goods_without_plans.insert(consumer_good);
    log_inventory_level(good, input_inventory[good]);
    log_catalog_addition(product);
}

int Distributor::try_sell_goods(ConsumerGood * consumer_good, int quantity, Person * person) {
    Good * good = consumer_good->corresponding_good;
    int available = std::min(static_cast<int>(
                get_inventory_level(consumer_good)), quantity);
    if (!available) {
        return 0;
    }
    if (available < quantity) {
        log_shortfall(consumer_good->id, quantity - available);
    }
    double cost = available * consumer_good->price_per_unit;
    if (!person->charge(cost)) {
         return 0;
    } 
    add_demand_signal(good, available);
    remove_input_from_inventory(consumer_good, available);
    check_and_reorder_input(good);
    return available;
}

void Distributor::end_plan(Plan * plan) {
    Firm::end_plan(plan);
    ConsumerGood * consumer_good = static_cast<ConsumerGood *>(plan->order->product);
    consumer_goods_without_plans.insert(consumer_good);
    renew_distribution_plan(consumer_good);
}

void Distributor::renew_distribution_plan(ConsumerGood * consumer_good) {
    Good * good = consumer_good->corresponding_good;
    double reorder_threshold = get_reorder_threshold(good);
    int distribution_quantity = std::min(
            get_inventory_level(good),
            reorder_threshold * FIRM_REORDER_MAX_PROP
            );
    if (!distribution_quantity) {
        return;
    }
    double distribution_time = 
        static_cast<double>(distribution_quantity)
        / demands[good];
    Order * order = new Order(
            consumer_good,
            distribution_quantity,
            this,
            distribution_time
            );
    if (Plan * plan = draft_plan_for_order(order)) {
        product_to_outbound_orders[consumer_good].insert(order);
        consumer_goods_without_plans.erase(consumer_good);
        start_plan(plan);
    }
}

double Distributor::get_pending_inventory_level(Product * product) {
    if (Good * good = dynamic_cast<Good *>(product)) {
        ConsumerGood * consumer_good = good->corresponding_consumer_good;
        return Firm::get_pending_inventory_level(good)
            + Firm::get_pending_inventory_level(consumer_good);
    }
    return Firm::get_pending_inventory_level(product);
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

