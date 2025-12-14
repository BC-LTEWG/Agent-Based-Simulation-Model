#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

#include "Distributor.h"
#include "Firm.h"
#include "Person.h"
#include "Product.h"
#include "Producer.h"
#include "Machine.h"
#include "Society.h"
#include "Sim.h"

Society * Society::instance = nullptr;

Society::Society() {
    instance = this;
    set_initial_products();
    set_initial_machines();
    // note: no way to assign products to producers or suppliers to distributors yet
    for (int i = 0; i < STARTING_NUM_PRODUCERS; i++) {
        Producer * producer = new Producer();
        std::vector<Machine*> shuffled = machines;
        std::shuffle(shuffled.begin(), shuffled.end(), Sim::gen);
        std::uniform_int_distribution<> count_dist(0, static_cast<int>(shuffled.size()));
        int assign_count = count_dist(Sim::gen);
        producer->machines.insert(
            producer->machines.end(),
            shuffled.begin(),
            shuffled.begin() + assign_count
        );

        producers.push_back(producer);
        firms.push_back(producer);
    }
    for (int i = 0; i < STARTING_NUM_DISTRIBUTORS; i++) {
        Distributor * distributor = new Distributor();
        distributors.push_back(distributor);
        firms.push_back(distributor);
    }
    // people MUST come after products and distributors are created
    for (int i = 0; i < STARTING_NUM_PEOPLE; i++) {
        birth_person();	
    }
}

void Society::set_initial_products() {
    for (int i = 0; i < STARTING_NUM_PRODUCTS; i++) {
        products.push_back(new Product("Product " + std::to_string(i)));
    }
    for (int i = NUM_BASE_PRODUCTS; i < STARTING_NUM_PRODUCTS; i++) {
        products[i]->set_inputs(products, i);
    }
}

void Society::set_initial_machines() {
    for (int i = 0; i < STARTING_NUM_MACHINES; i++) {
        machines.push_back(new Machine("Machine " + std::to_string(i), 1000));
    }
}

std::vector<Product *>& Society::get_products() {
    return products;
}

std::vector<Machine *>& Society::get_machines() {
    return machines;
}
std::vector<Distributor *>& Society::get_distributors() {
    return distributors;
}

std::vector<Producer *>& Society::get_producers() {
    return producers;
}

std::vector<Firm *>& Society::get_firms() {
    return firms;
}

std::vector<Person *>& Society::get_unemployed_people() {
    return unemployed_people;
}

int Society::get_current_work_hours_daily() {
    return current_work_hours_daily;
}

Person * Society::birth_person() {
    Person * person = new Person();
    people.push_back(person);
    unemployed_people.push_back(person);
    return person;
}

void Society::retire_person(Person * person) {
    // unimplemented until hiring/reallocation is done
}

