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
            (FIRM_STOCKPILE_DURATION + FIRM_DEMAND_WINDOW * FIRM_INITIAL_INVENTORY_MULT) * 
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
}

void Distributor::sell_goods(Product& product, int quantity, Person * person) {
    add_demand_signal(&product, quantity);

    bool has_inventory = inventory.count(&product) > 0;
    int available = has_inventory ? inventory[&product] : 0;
    if (!has_inventory) {
        Logger::get_instance()->log(
                Logger::DISTRIBUTOR,
                "inventory_missing",
                id,
                product.product_name,
                0
                );
    }
    int remainder = 0;
    int sell_quantity = std::min(available, quantity);
    if (available < quantity) {
        remainder = quantity - available;
        Logger::get_instance()->log(
                Logger::DISTRIBUTOR,
                "shortfall",
                id,
                product.product_name,
                remainder
                );
    }
    if (sell_quantity == 0) {
        return;
    }
    ConsumerGood * consumer_good = society->get_consumer_good(&product);
    if (!consumer_good) {
        std::cerr << "No consumer good for product " << product.product_name << std::endl;
        return;
    }
    double cost = sell_quantity * consumer_good->price_per_unit;
    if (!person->charge(cost)) {
        std::cerr << "Person cannot afford " << sell_quantity
            << " units of " << product.product_name << " costing " 
            << cost << std::endl;
        return;
    } 
    Plan * plan = product_to_plan[&product];
    plan->outgoing_units_consumed += sell_quantity;
    plan->prd += cost;
    this->pooled_account -= cost;
    inventory[&product] -= sell_quantity;
}

std::unordered_set<Product *> Distributor::get_products_to_reorder() {
    return catalog;
}
