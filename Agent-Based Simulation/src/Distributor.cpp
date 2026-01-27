#include <iostream>
#include <climits>
#include "Constants.h"
#include "Distributor.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"

Distributor::Distributor() : Firm() {}

Distributor::Distributor(std::unordered_set<Product *> initial_catalog) :
    Firm(initial_catalog) {
    for (Product * p : initial_catalog) {
        inventory[p] = p->order_size * FIRM_INITIAL_INVENTORY_MULTIPLIER;
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

void Distributor::sell_goods(Product& product, int quantity, Person * person) {
    add_demand_signal(&product, quantity);

    if (!inventory[&product]) {
        std::cerr << "Inventory has no such product: " << product.product_name << std::endl;
    }
    int available = inventory[&product];
    int remainder = 0;
    if (available >= quantity) {
        inventory[&product] -= quantity;
    } else {
        inventory[&product] = 0;
        remainder = quantity - available; 
        std::cout << "Shortfall in product " << product.product_name 
            << " of " << remainder << " units. " << std::endl;
        
        int turnaround_time = product.living_labor_per_unit * (quantity);
        std::unique_ptr<Order> new_order =
            std::make_unique<Order>(&product, quantity, this, turnaround_time); 
        
    }
    double cost = (available - remainder) * product.price_per_unit;
    person->charge(cost);
}





std::unordered_set<Product *> Distributor::get_products_to_reorder() {
    return catalog;
}
