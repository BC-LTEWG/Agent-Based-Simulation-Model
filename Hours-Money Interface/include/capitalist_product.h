#pragma once

#include <string>
#include <vector>

struct CapitalistProduct
{
    std::string name;
    std::string company;
    double price;

    void generate_name();
    void new_desired_price();

    // How much it would sell
    double sell_factor;
    double total_amount;

    void new_sell_factor();
    void new_total_amount();

    // How much of the price is surplus value charged by the capitalist company
    // Note that we are assuming the same surplus factor for the capitalist market and
    // the export to the labor time economy
    double surplus_factor;

    void new_surplus_factor();
};