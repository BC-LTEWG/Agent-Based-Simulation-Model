#include <cmath>
#include <Eigen/Dense>
#include <iostream>
#include <numeric>
#include <unordered_map>

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
        Producer * producer = new Producer({products[i % STARTING_NUM_PRODUCTS]});
        producers.push_back(producer);
        firms.push_back(producer);
    }
    for (int i = 0; i < STARTING_NUM_DISTRIBUTORS; i++) {
        Distributor * distributor = new Distributor({products[i % STARTING_NUM_PRODUCTS]});
        distributors.push_back(distributor);
        firms.push_back(distributor);
    }
    // add suppliers to firms
    for (Firm * firm : firms) {
        for (Producer * producer : producers) {
            if (producer != firm) {
                firm->add_supplier(producer);
            }
        }
    }
    // people MUST come after products and distributors are created
    for (int i = 0; i < STARTING_NUM_PEOPLE; i++) {
        birth_person();	
    }
}

void Society::set_initial_products() {
    for (std::size_t i = 0; i < STARTING_NUM_PRODUCTS; ++i) {
        products.push_back(new Product("Product " + std::to_string(i)));
    }
    for (Product * product: products) {
        product->set_inputs(products);
    }
    set_product_prices();
}

void Society::set_product_prices() {
    std::unordered_map<Product *, size_t> product_to_index;
    for (size_t i = 0; i < products.size(); ++i) {
        product_to_index[products[i]] = i;
    }
    size_t dim = products.size();
    Eigen::MatrixXd A_t(dim, dim);
    Eigen::VectorXd l(dim);
    for (Product * product : products) {
        for (const std::pair<Product * const, double>& input :
                product->inputs_per_unit) {
            A_t(product_to_index[product], product_to_index[input.first]) =
                input.second;
        }
        l(product_to_index[product]) = product->living_labor_per_unit;
    }
    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(dim, dim);
    Eigen::MatrixXd Leontief = I - A_t;
    Eigen::MatrixXd Leontief_inv = Leontief.inverse();
    Eigen::VectorXd values = Leontief_inv * l;
    for (std::size_t i = 0; i < dim; ++i) {
        if (values(i) < 0.0) {
            std::cerr << "Value " << i << " has a negative value: " <<
                values(i) << std::endl;
        }
        products[i]->price_per_unit = values(i);
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

