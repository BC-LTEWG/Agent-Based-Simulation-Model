#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "Firm.h"
#include "Person.h"
#include "Product.h"
#include "Machine.h"
#include "Producer.h"
#include "Distributor.h"


int main() {
    std::cout << "=== Testing Re-ordering System ===" << std::endl << std::endl;

    std::cout << "Creating products..." << std::endl;
    Product* bread = new Product("Bread", 5.0, 100);
    Product* milk = new Product("Milk", 3.0, 50);
    std::cout << "Product 1: " << bread->product_name << " - $" << bread->price_per_unit << " (order: " << bread->order_size << ")" << std::endl;
    std::cout << "Product 2: " << milk->product_name << " - $" << milk->price_per_unit << " (order: " << milk->order_size << ")" << std::endl << std::endl;

    std::cout << "Creating workers..." << std::endl;
    std::unordered_map<std::string, int> expertise;
    std::unordered_map<std::string, double> needs;
    
    std::vector<Person*> bakery_workers;
    for (int i = 0; i < 5; ++i) {
        bakery_workers.push_back(new Person(expertise, 30, Person::HEALTHY, needs));
    }
    
    std::vector<Person*> dairy_workers;
    for (int i = 0; i < 3; ++i) {
        dairy_workers.push_back(new Person(expertise, 28, Person::HEALTHY, needs));
    }
    
    std::vector<Person*> distributor_workers;
    for (int i = 0; i < 2; ++i) {
        distributor_workers.push_back(new Person(expertise, 25, Person::HEALTHY, needs));
    }
    
    std::cout << "Created " << (bakery_workers.size() + dairy_workers.size() + distributor_workers.size()) << " workers" << std::endl << std::endl;

    std::cout << "Creating machines..." << std::endl;
    std::vector<Machine*> bakery_machines;
    bakery_machines.push_back(new Machine("Oven", 10000));
    
    std::vector<Machine*> dairy_machines;
    dairy_machines.push_back(new Machine("Pasteurizer", 10000));
    
    std::cout << "Created " << (bakery_machines.size() + dairy_machines.size()) << " machines" << std::endl << std::endl;

    std::cout << "Creating producers..." << std::endl;
    Producer* bakery = new Producer(bakery_machines, bakery_workers);
    bakery->add_product_to_catalog(bread);
    std::cout << "Bakery created with " << bakery_workers.size() << " workers" << std::endl;
    
    Producer* dairy = new Producer(dairy_machines, dairy_workers);
    dairy->add_product_to_catalog(milk);
    std::cout << "Dairy created with " << dairy_workers.size() << " workers" << std::endl << std::endl;

    std::cout << "Creating distributor..." << std::endl;
    Distributor* store = new Distributor(std::vector<Machine*>(), distributor_workers, std::vector<Plan*>());
    store->add_supplier(bakery);
    store->add_supplier(dairy);
    std::cout << "Store created with " << distributor_workers.size() << " workers and 2 suppliers" << std::endl << std::endl;

    std::cout << "Setting initial inventory..." << std::endl;
    store->initialize_inventory(bread, 150);
    store->initialize_inventory(milk, 80);
    std::cout << "  Bread inventory: " << store->get_inventory(bread) << std::endl;
    std::cout << "  Milk inventory: " << store->get_inventory(milk) << std::endl << std::endl;

    std::cout << "Simulating sales (reducing inventory)..." << std::endl;
    store->initialize_inventory(bread, 90);
    std::cout << "  Sold bread, new inventory: " << store->get_inventory(bread) << " (below threshold of " << bread->order_size << ")" << std::endl;
    store->initialize_inventory(milk, 40);
    std::cout << "  Sold milk, new inventory: " << store->get_inventory(milk) << " (below threshold of " << milk->order_size << ")" << std::endl << std::endl;

    std::cout << "Checking for re-orders..." << std::endl;
    store->check_and_reorder();
    std::cout << std::endl;

    std::cout << "Processing producer orders over multiple days..." << std::endl;
    for (int day = 1; day <= 5; ++day) {
        std::cout << "Day " << day << ":" << std::endl;
        bakery->process_orders();
        dairy->process_orders();
    }
    std::cout << std::endl;

    std::cout << "Final inventory status:" << std::endl;
    std::cout << "  Bread inventory: " << store->get_inventory(bread) << std::endl;
    std::cout << "  Milk inventory: " << store->get_inventory(milk) << std::endl << std::endl;

    std::cout << "=== All tests completed successfully! ===" << std::endl;

    delete bread;
    delete milk;
    for (auto worker : bakery_workers) delete worker;
    for (auto worker : dairy_workers) delete worker;
    for (auto worker : distributor_workers) delete worker;
    for (auto machine : bakery_machines) delete machine;
    for (auto machine : dairy_machines) delete machine;
    delete bakery;
    delete dairy;
    delete store;
    
    return 0;
}
