#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Constants.h"

struct Product {
    Product(const std::string name);
    void set_inputs();
    std::string product_name;
    double price_per_unit;
    int order_size;
    std::unordered_map<Product *, int> inputs_per_unit;
 	double living_labor_per_unit; 
	std::vector<Ability> required_abilities;
	double mean_consumption_frequency;
};
