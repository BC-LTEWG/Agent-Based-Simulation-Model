#include <algorithm>
#include <climits>
#include <iostream>
#include <string>

#include "ConsumerGood.h"
#include "Distributor.h"
#include "Logger.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"
#include "Society.h"

Distributor::Distributor(Society * society) : Firm{society} {}

Distributor::Distributor(
        Society * society,
        std::unordered_set<Product *> initial_catalog
        ) :
    Firm(society, initial_catalog)
{
    for (Product * product : get_products_to_reorder()) {
        society->add_consumer_good(product);
        int quantity =
            product->mean_consumption_frequency *
            (FIRM_STOCKPILE_DURATION + FIRM_DEMAND_WINDOW_MIN * DISTRIBUTOR_INITIAL_INVENTORY_MULT) * 
            STARTING_NUM_PEOPLE;
        Order * order = new Order(product, quantity, this, 0);
        Plan * plan = new Plan;
        plan->order = order;
        plan->firm = this;
        plan->training_time_remaining = plan->training_time = 0;
        plan->predicted_turnaround_time = 0;
        plan->labor_hours = DISTRIBUTION_LABOR_PER_UNIT * quantity;
        plan->raw_materials_remaining = plan->raw_materials = 0.0;
        plan->total_hours_remaining = plan->total_hours =
            plan->labor_hours + plan->raw_materials;
        plan->prd = -(plan->total_hours);
        plan->outgoing_units_consumed = 0;
        plan->machinery_cost = 0.0;
        plans_in_progress.push_back(plan);
        product_to_plan[product] = plan;
        inventory[product] = quantity;
    }
}

void Distributor::on_time_step() {
    Firm::on_time_step();
    for (Plan * plan : plans_in_progress) {
        plan->labor_hours_remaining -= plan->workers.size();
        for (Person * worker : plan->workers) {
            worker->register_hours_worked(1);
        }
    }
    check_expand_catalog();
    log_catalog_size(catalog.size());
}

bool Distributor::try_sell_goods(Product& product, int quantity, Person * person) {
    ConsumerGood * consumer_good = society->get_consumer_good(&product);
    if (!consumer_good) {
        std::cerr << "No consumer good for product " << product.product_name << std::endl;
        return false;
    }

    add_demand_signal(&product, quantity);

    int available = catalog.count(&product) ? inventory[&product] : 0;
    if (available < quantity) {
        log_shortfall(product.product_name, quantity - available);
        return false;
    }

    double cost = quantity * consumer_good->price_per_unit;
    if (!person->charge(cost)) {
        std::cerr << "Person cannot afford " << quantity
            << " units of " << product.product_name << " costing " 
            << cost << std::endl;
        return false;
    } 

    Plan * plan = product_to_plan[&product];
    plan->outgoing_units_consumed += quantity;
    plan->prd += cost;
    inventory[&product] -= quantity;
    return true;
}

std::unordered_set<Product *> Distributor::get_products_to_reorder() {
    return catalog;
}

void Distributor::check_expand_catalog() {
    for (Product * product : get_products_to_reorder()) {
        if (get_demand(product) > EXPAND_CATALOG_DEMAND_THRESHOLD && !catalog.count(product)) {
            catalog.insert(product);
        }
    }
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

void Distributor::log_catalog_size(int size) {
    Logger::get_instance()->log(
            Logger::DISTRIBUTOR,
            "catalog_size",
            id,
            size
            );
}
