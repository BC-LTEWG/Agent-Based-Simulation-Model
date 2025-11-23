#pragma once

#include <string>
#include <vector>

#include "Constants.h"

struct Product {
    std::string product_name;
    double price_per_unit;
    int order_size;
	double base_frequency;
	std::vector<Ability> required_abilities;
  
    Product(const std::string name, double price, int order);
};
