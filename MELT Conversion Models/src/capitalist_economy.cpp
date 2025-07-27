#include "capitalist_economy.h"
#include "values.h"
#include <iostream>

void CapitalistEconomy::set_value(const std::string &field_name, double value)
{
    double final_value = (value < 0) ? INVALID_VALUE : value;

    if (field_name == "gdp_nominal")
    {
        gdp_nominal_per_capita = final_value;
    }
    else if (field_name == "gdp_ppp")
    {
        gdp_ppp_per_capita = final_value;
    }
    else if (field_name == "median_income")
    {
        median_income = final_value;
    }
    else if (field_name == "mean_income")
    {
        mean_income = final_value;
    }
    else if (field_name == "hours")
    {
        avg_work_hours_per_year = final_value;
    }
    else
    {
        std::cerr << "Invalid field name: " << field_name << std::endl;
    }
}
