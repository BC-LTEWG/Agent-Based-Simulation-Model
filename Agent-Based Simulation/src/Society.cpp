#include <cmath>
#include <numeric>

#include "Distributor.h"
#include "Firm.h"
#include "Person.h"
#include "Product.h"
#include "Producer.h"
#include "Society.h"

Society * Society::instance = nullptr;

Society::Society() {
    instance = this;
    set_initial_products();
    // note: no way to assign products to producers or suppliers to distributors yet
    for (int i = 0; i < STARTING_NUM_PRODUCERS; i++) {
        Producer * producer = new Producer();
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

std::vector<Product *>& Society::get_products() {
    return products;
}

std::vector<Distributor *>& Society::get_distributors() {
    return distributors;
}

std::vector<Person *>& Society::get_unemployed_people() {
    return unemployed_people;
}

int Society::get_current_work_hours_daily() {
    return current_work_hours_daily;
}

int Society::get_current_work_days_weekly() {
	return current_work_days_weekly;
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

