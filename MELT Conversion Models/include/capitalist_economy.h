#pragma once
#include <string>
#include "values.h"

struct CapitalistEconomy
{
    int ce_id;
    std::string economy_name;

    double gdp_nominal_per_capita = INVALID_VALUE;
    double gdp_ppp_per_capita = INVALID_VALUE;
    double median_income = INVALID_VALUE;
    double mean_income = INVALID_VALUE;

    double avg_work_hours_per_year = INVALID_VALUE;

    // Set a value for a given field name
    void set_value(const std::string &field_name, double value);
};
