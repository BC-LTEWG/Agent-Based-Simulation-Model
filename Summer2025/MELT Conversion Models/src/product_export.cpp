#include "product_export.h"
#include "values.h"
#include <iostream>

void ProductExport::set_value(const std::string &field_name, double value)
{
    double final_value = (value < 0) ? INVALID_VALUE : value;

    if (field_name == "labor_time_value")
    {
        labor_time_value = final_value;
    }
    else
    {
        std::cerr << "Invalid field name: " << field_name << std::endl;
    }
}
