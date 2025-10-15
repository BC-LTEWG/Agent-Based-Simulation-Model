#pragma once

#include <string>
#include <vector>
#include "labor_time_economy.h"

struct CapitalistCompany;

struct CapitalistProduct
{
    std::string product_name;
    CapitalistCompany *company = nullptr;
    char product_type; // product type can be car or laptop in reality, an example will be cars produced by different firms currently in the market
    double price;

    void set_product_name();
    void set_product_type();
    void set_desired_price();
    void export_to_destined_economy(const LaborTimeEconomy &lte);

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