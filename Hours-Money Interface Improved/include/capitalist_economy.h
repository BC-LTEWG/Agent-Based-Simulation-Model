#pragma once

#include <string>
#include <vector>
#include <map>
#include "capitalist_company.h"
#include "capitalist_product.h"
#include "values.h"

struct CapitalistEconomy
{
    std::string name;
    double curreny_amount = 0;                                // how much money is currently being held
    double earnings_per_capita = INITIAL_EARNINGS_PER_CAPITA; // value used in melt calculation
    double work_hours = INITIAL_HOURS;                        // how many hours worked on average in a year
    double melt;                                              // what would the new melt be in a new cycle
    std::vector<CapitalistCompany> companies;
    std::vector<CapitalistProduct> market; // each new product will be added to the market with its market price determined
    std::map<char, double> market_price;   // each type of product has its own average market price

    void new_earnings_per_capita();
    void new_work_hours();
    void new_melt();
    void add_product(CapitalistProduct &product);
    void remove_product(CapitalistProduct &product);

    // This should be the last thing to do every cycle
    // It updates the market price of every type of product
    void adjust_market_price();

    // How likely is an economy is going to create a new company
    // or remove an existing company at the end of the cycle
    double create_factor;
    double remove_factor;

    void new_create_factor();
    void new_remove_factor();
};