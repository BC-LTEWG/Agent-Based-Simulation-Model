#include <iostream>
#include <climits>
#include "Distributor.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"
#include "Product.h"

Distributor::Distributor() : Firm() {}

void Distributor::on_time_step() {
    for (auto iter = plans_in_progress.begin(); iter != plans_in_progress.end(); ++iter) {
        Plan * plan = *iter;
        
        plan->labor_hours_remaining -= plan->workers.size();
        plan->prd += plan->workers.size();
        
        for (auto * worker : plan->workers) {
            worker->register_hours_worked(1);
        }
        

        double m = (static_cast<double>(plan->labor_hours) / plan->workers.size()) * machines.size();

        double hours_per_machine = m / machines.size();
        for (auto * machine : machines) {
            machine->hours_used -= hours_per_machine;
        }
        
        plan->prd += m + (plan->labor_hours - plan->labor_hours_remaining);
        
        delete plan;
        
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
    }
    double cost = (available - remainder) * product.price_per_unit;
    person->charge(cost);
}




bool Distributor::is_overproduced(Product* product) {
    for(auto& products : plans_in_progress) {
        if(products->order->product == product) {
            return products->order->quantity > PRODUCTION_THRESHOLD * products->order->quantity;
        }
    }
    return false;
}


