#pragma once

#include <string>
#include <vector>
#include "capitalist_company.h"
#include "capitalist_product.h"
#include "values.h"

struct CapitalistEconomy
{
    std::string name;
    double curreny_amount;             // how much money is currently being held
    double economic_indicator;         // value used in melt calculation
    double work_hours = INITIAL_HOURS; // how many hours worked on average in a year
    double melt;                       // what would the new melt be in a new cycle
    std::vector<CapitalistCompany> companies;
    std::vector<CapitalistProduct> market; // each new product will be added to the market with market price determined

    void new_indicator();
    void new_work_hours();
    void new_melt();
    void add_product(CapitalistProduct &product);
    void remove_product(const std::string &name);

    void adjust_market();

    // How likely is an economy is going to create a new company
    // or remove an existing company at the end of the cycle
    double create_factor;
    double remove_factor;

    void new_create_factor();
    void new_remove_factor();
};