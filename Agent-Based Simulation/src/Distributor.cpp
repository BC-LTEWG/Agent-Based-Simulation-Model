#include <iostream>
#include <climits>
#include "Distributor.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"

Distributor::Distributor() : Firm() {}

Distributor::Distributor(std::unordered_set<Product *> initial_catalog) :
    Firm(initial_catalog) {
    for (Product * p : get_products_to_reorder()) {
        inventory[p] = get_reorder_threshold(p) * FIRM_INITIAL_INVENTORY_MULTIPLIER;
    }
}

void Distributor::on_time_step() {
    Firm::on_time_step();
    for (Plan * current_plan_in_progress : plans_in_progress) {
        Plan * plan = current_plan_in_progress;
        plan->labor_hours_remaining -= plan->workers.size();
        for (Person * worker : plan->workers) {
            worker->register_hours_worked(1);
        }
        if (plan->outgoing_units_consumed == plan->order->quantity) {
            plan->prd = plan->total_hours; 
        }
    }
}

double Distributor::get_output_ratio(Product& product) {
    return 1.0 / product.order_size;
}

double Distributor::planned_satisfaction_per_person(Product& product, Person& person) {
    return get_output_ratio(product) * person.get_purchase_frequencies()[&product];
}

void Distributor::sell_goods(Product& product, int quantity, Person * person) {
    add_demand_signal(&product, quantity);

    if (!inventory[&product]) {
        std::cerr << "Inventory has no such product: " << product.product_name << std::endl;
        return;
    }
    int available = inventory[&product];
    if (available < quantity) {
        std::cerr << "Shortfall in product " << product.product_name 
            << " of " << quantity - available << " units. " << std::endl;
        return;
    }
    int sell_quantity = std::min(available, quantity);
    double cost = sell_quantity * product.price_per_unit;
    if (!person->charge(cost)) {
        std::cerr << "Person cannot afford " << sell_quantity 
            << " units of " << product.product_name << " costing " 
            << cost << std::endl;
        return;
    } 
    inventory[&product] -= std::min(available, quantity);
}

bool Distributor::is_overproduced(Product* product) {
    for(auto& goods : plans_in_progress) {
        if(goods->order->product == product) {
            return goods->order->quantity > PRODUCTION_THRESHOLD * goods->order->quantity;
        }
    }
    return false;
}

std::unordered_set<Product *> Distributor::get_products_to_reorder() {
    return catalog;
}
