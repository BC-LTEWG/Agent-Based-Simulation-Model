#pragma once

#include <string>
#include <vector>
#include "labor_time_company.h"

struct LaborTimeEconomy
{
    std::string name;
    double currency_amount = 0; // how much money is currently being held
    std::vector<LaborTimeCompany> companies;

    // How likely is an economy is going to create a new company
    // or remove an existing company at the end of the cycle
    double create_factor;
    double remove_factor;

    void new_create_factor();
    void new_remove_factor();
};