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
        Good * good = static_cast<ConsumerGood *>(product)->corresponding_good;
        input_inventory[good] =
            good->corresponding_consumer_good->mean_consumption_frequency *
            (FIRM_STOCKPILE_DURATION + FIRM_DEMAND_WINDOW_MIN) * 
            Sim::get_num_people() / Sim::get_num_distributors();
        log_inventory_level(good, input_inventory[good]);
    }
}

Logger::Client Distributor::get_client_type() {
    return Logger::DISTRIBUTOR;
}

void Distributor::on_time_step() {
    Firm::on_time_step();
}

int Distributor::try_sell_goods(ConsumerGood * consumer_good, int quantity, Person * person) {
    add_demand_signal(consumer_good->corresponding_good, quantity);
    check_and_reorder_input(consumer_good->corresponding_good);
    if (!catalog.count(consumer_good)) return 0;
    int available = std::min(static_cast<int>(
                get_inventory_level(consumer_good->corresponding_good)), quantity);
    if (available < quantity) {
         log_shortfall("", quantity - available);
    }
    double cost = available * consumer_good->price_per_unit;
    if (!person->charge(cost)) {
         return 0;
    } 
    remove_input_from_inventory(consumer_good->corresponding_good, quantity);
    return available;
}

void Distributor::log_shortfall(std::string product_name, int shortfall) {
    Logger::get_instance()->log(
            Logger::DISTRIBUTOR,
            "shortfall",
            id,
            product_name,
            shortfall
            );
}

