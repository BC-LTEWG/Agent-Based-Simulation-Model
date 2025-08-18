#pragma once

#include <capitalist_economy.h>
#include <string>
#include <vector>

struct LaborTimeProduct
{
    std::string product_name;
    std::string company;
    char product_type;
    double labor_time;
    double sell_price;

    void set_product_name();
    void set_product_type();
    void set_labor_time();
    void calculate_sell_price(const CapitalistEconomy &economy);

    // How much it would sell
    double portion_sold_in_export;
    double quantity_produced_for_export;

    void set_portion_sold_in_export();
    void set_quantity_produced_for_export();

    // How much surplus value will be charged by the labor time company when exporting
    // Assume that there is no market price available so this will be factor times labor_time times melt
    double surplus_factor;

    void set_surplus_factor();
};