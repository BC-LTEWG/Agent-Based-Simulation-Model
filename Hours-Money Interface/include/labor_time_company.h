#pragma once

#include <string>
#include <vector>
#include "labor_time_product.h"

struct LaborTimeCompany
{
    std::string name;
    std::string economy;
    std::vector<LaborTimeProduct> products;

    std::string print_products();

    // How likely is a company going to create a new product
    // or remove an existing product at the end of the cycle
    double create_factor;
    double remove_factor;

    void new_create_factor();
    void new_remove_factor();
};
