#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Constants.h"

struct Product {
    Product(const std::string name, double price, int order);
    int get_required_labor();
    void add_input(Product * input, int quantity);
    std::string product_name;
    double price_per_unit;
    int order_size;
	double base_frequency;
    std::unordered_map<Product *, int> inputs;
	std::vector<Ability> required_abilities;
  
};
