#pragma once

#include <string>
#include <vector>
#include "capitalist_product.h"

struct CapitalistEconomy;
struct CapitalistProduct;

struct CapitalistCompany
{
    std::string company_name;
    CapitalistEconomy *economy = nullptr;
    std::vector<CapitalistProduct> products;

    void set_company_name();
    void create_product();
    std::string print_products();

    // How likely is a company going to create a new product
    // or remove an existing product at the end of the cycle
    double create_factor;
    double remove_factor;

    void new_create_factor();
    void new_remove_factor();

    CapitalistCompany(CapitalistEconomy *ce);
};
