#include <iostream>

#include "Distributor.h"

Distributor::Distributor() : Firm() {}

Distributor::Distributor(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) 
    : Firm(machines, workforce, plans) {}

double Distributor::get_output_ratio(Product& product) {
    return 1.0 / product.order_size;
}

double Distributor::planned_satisfaction_per_person(Product& product, Person& person) {
    return get_output_ratio(product) * person.get_purchase_frequencies()[&product];
}

void Distributor::sell_goods(Product& product, int quantity, Person * person) {
    if (!inventory[&product]) {
        std::cerr << "Inventory has no such product: " << product.product_name << std::endl;
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
}
