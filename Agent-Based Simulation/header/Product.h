#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Person.h"

struct Machine;

struct Product {
    Product(const std::string name);
    void set_inputs(std::vector<Product *>& products, int product_index);
    std::string product_name;
    double price_per_unit;
    int order_size;
    std::vector<Machine *> machines_needed;
    std::unordered_map<Product *, double> inputs_per_unit;
 	double living_labor_per_unit; 
	std::vector<Person::Ability> required_abilities;
	double mean_consumption_frequency;
};
