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
        Society * society,
        const std::unordered_set<Product *>& initial_catalog
        ) :
    Firm(society, initial_catalog)
{
    for (Product * product : catalog) {
        ConsumerGood * consumer_good = static_cast<ConsumerGood *>(product);
        Good * good = consumer_good->corresponding_good;
        demands[good] = 
            consumer_good->mean_consumption_frequency 
            * Sim::get_num_people() 
            / Sim::get_num_distributors();
        input_inventory[good] =
            input_inventory[consumer_good] =
            demands[good] * FIRM_STOCKPILE_DURATION / 2;
        log_inventory_level(good, input_inventory[good]);
        consumer_goods_without_plans.insert(consumer_good);
    }
    log_catalog();
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
    check_and_reorder_input(good);
    remove_input_from_inventory(consumer_good, available);
    return available;
}

void Distributor::renew_distribution_plan(ConsumerGood * consumer_good) {
    Good * good = consumer_good->corresponding_good;
    if (get_inventory_level(good)) {
        input_inventory[consumer_good] += get_inventory_level(good);
        input_inventory[good] = 0;
    }
}

double Distributor::get_pending_inventory_level(Product * product) {
    Good * good = static_cast<Good *>(product);
    ConsumerGood * consumer_good = good->corresponding_consumer_good;
    return Firm::get_pending_inventory_level(good)
        + Firm::get_pending_inventory_level(consumer_good);
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

