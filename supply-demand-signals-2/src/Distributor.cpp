#include <iostream>

#include "Distributor.h"
#include <iostream>

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

void Distributor::add_supplier(Producer* producer) {
    suppliers.push_back(producer);
}

void Distributor::set_reorder_threshold(Product* product, int threshold) {
    reorder_thresholds[product] = threshold;
}

void Distributor::check_and_reorder() {
    for (auto& pair : inventory) {
        Product* product = pair.first;
        int current_inventory = pair.second;
        
        int threshold = product->order_size;
        if (reorder_thresholds.find(product) != reorder_thresholds.end()) {
            threshold = reorder_thresholds[product];
        }
        
        if (current_inventory < threshold) {
            Producer* producer = find_producer_for_product(product);
            if (producer) {
                int order_quantity = product->order_size;
                std::cout << "Reordering " << order_quantity << " units of " 
                          << product->product_name << std::endl;
                Order* order = producer->accept_order(product, order_quantity, this);
                if (order) {
                    std::cout << "Order accepted. Turnaround time: " 
                              << order->turnaround_time << " days" << std::endl;
                }
            } else {
                std::cerr << "No producer found for product: " 
                          << product->product_name << std::endl;
            }
        }
    }
}

Producer* Distributor::find_producer_for_product(Product* product) {
    for (Producer* producer : suppliers) {
        if (producer->can_produce(product)) {
            return producer;
        }
    }
    return nullptr;
}

void Distributor::receive_order(Product* product, int quantity) {
    inventory[product] += quantity;
    std::cout << "Received " << quantity << " units of " 
              << product->product_name << ". New inventory: " 
              << inventory[product] << std::endl;
}

void Distributor::initialize_inventory(Product* product, int quantity) {
    inventory[product] = quantity;
}

int Distributor::get_inventory(Product* product) {
    return inventory[product];
}
