#include "product_import.h"
#include "values.h"
#include <iostream>

void ProductImport::set_value(const std::string &field_name, double value)
{
    double final_value = (value < 0) ? INVALID_VALUE : value;

    if (field_name == "currency_value")
    {
        currency_value = final_value;
    }
    else
    {
        std::cerr << "Invalid field name: " << field_name << std::endl;
    }
}
