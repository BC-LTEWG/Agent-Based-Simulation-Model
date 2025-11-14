#pragma once

#include <string>
#include <vector>

struct Product {
    std::string product_name;
    double price_per_unit;
    int order_size;
	double base_frequency;
  
    Product(const std::string name, double price, int order);
};
