#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct Product {
    std::string product_name;
    double price_per_unit;
    int order_size;
	double base_frequency;

    std::vector<std::vector<std::pair<Product*, double>>> leontief_matrix;
  
    Product(const std::string name, double price, int order);
    int get_required_labor();
    void set_leontief_matrix();
};
