#pragma once

#include <string>
#include <vector>
#include "capitalist_economy.h"

struct LaborTimeCompany;
struct CapitalistEconomy;

struct LaborTimeProduct
{
    std::string product_name;
    LaborTimeCompany *company = nullptr;
    char product_type;
    double labor_time;
    double sale_price;

    void set_product_name();
    void set_product_type();
    void set_labor_time();
    void calculate_sale_price(const CapitalistEconomy &economy);
    void export_to_destined_economy(const CapitalistEconomy &economy);

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