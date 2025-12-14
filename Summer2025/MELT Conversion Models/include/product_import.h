#pragma once
#include <string>
#include "values.h"

struct ProductImport
{
    int product_id;
    std::string product_name;
    double currency_value = INVALID_VALUE; // Currency cost needed for import

    void set_value(const std::string &field_name, double value);
};
