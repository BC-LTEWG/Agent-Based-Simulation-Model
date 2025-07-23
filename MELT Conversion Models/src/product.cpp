#include "product.h"
#include "values.h"

Product::Product(int id, const std::string &name, double ltv, double cv)
    : product_id(id), product_name(name),
      labor_time_value(ltv), currency_value(cv) {}

Product Product::for_export(int id, const std::string &name, double labor_hours)
{
    return Product(id, name, labor_hours, INVALID_VALUE);
}

Product Product::for_import(int id, const std::string &name, double currency)
{
    return Product(id, name, INVALID_VALUE, currency);
}

bool Product::is_export() const
{
    return labor_time_value != INVALID_VALUE;
}

bool Product::is_import() const
{
    return currency_value != INVALID_VALUE;
}
