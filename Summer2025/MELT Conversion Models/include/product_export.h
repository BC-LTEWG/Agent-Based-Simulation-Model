#pragma once
#include <string>
#include "values.h"

struct ProductExport
{
    int product_id;
    std::string product_name;
    double labor_time_value = INVALID_VALUE; // Labor time needed for export (hours)

    void set_value(const std::string &field_name, double value);
};
