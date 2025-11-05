#include "Distributor.h"

Distributor::Distributor() : Firm() {}

Distributor::Distributor(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) 
    : Firm(machines, workforce, plans) {}

double Distributor::get_output_ratio(Product& product) {
    return 1.0 / product.order_size;
}

double Distributor::planned_satisfaction_per_person(Product& product, Person& person) {
    return get_output_ratio(product) * person.get_worker_needs()[product.product_name];
}

void Distributor::sell_goods(Product& product, int quantity) {
    if (!inventory[&product]) {
        std::cerr << "Inventory has no such product: " << product.name << std::endl;
        int available = inventory[&product];
        if (available >= quantity) {
            inventory[&product] -= quantity;
        } else {
            inventory[&product] = 0;
            int remainder = quantity - available;
            std::cout << "Shortfall in product " << product.name << " of " << remainder << " units. " << std::endl;
        }
}
