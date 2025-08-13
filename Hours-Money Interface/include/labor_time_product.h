#pragma once

#include <capitalist_economy.h>
#include <string>
#include <vector>

struct LaborTimeProduct
{
    std::string name;
    std::string company;
    char product_type;
    double labor_time;
    double sell_price;

    void generate_name();
    void generate_product_type();
    void new_labor_time();
    void calculate_sell_price(CapitalistEconomy economy);

    // How much it would sell
    double sell_factor;
    double total_amount;

    void new_sell_factor();
    void new_total_amount();

    // How much surplus value will be charged by the labor time company when exporting
    // Assume that there is no market price available so this will be factor times labor_time times melt
    double surplus_factor;

    void new_surplus_factor();
};