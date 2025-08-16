#pragma once

#include <string>
#include <vector>

struct CapitalistProduct
{
    std::string product_name;
    std::string company;
    char product_type;
    double price;

    void set_product_name();
    void set_product_type();
    void set_desired_price();

    double portion_sold_in_export;
    double quantity_produced_for_export;

    void set_portion_sold_in_export();
    void set_quantity_produced_for_export();

    // How much of the price is surplus value charged by the capitalist company
    // Note that we are assuming the same surplus factor for the capitalist market and
    // the export to the labor time economy
    double surplus_factor;

    void set_surplus_factor();
};