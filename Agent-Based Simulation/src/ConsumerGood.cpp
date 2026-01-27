#include "Constants.h"
#include "ConsumerGood.h"

ConsumerGood::ConsumerGood(Product * product) :
    Product(product->product_name)
{
    order_size = 1;
    inputs_per_unit[product] = 1;
    living_labor_per_unit = DISTRIBUTION_LABOR_PER_UNIT;
    price_per_unit = product->price_per_unit + living_labor_per_unit;
}

