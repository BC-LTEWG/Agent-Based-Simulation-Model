#include "Constants.h"
#include "ConsumerGood.h"

ConsumerGood::ConsumerGood(Product * product) {
    id = product->id;
    product_name = product->product_name;
    order_size = 1;
    living_labor_per_unit = DISTRIBUTION_LABOR_PER_UNIT;
    price_per_unit = product->price_per_unit + DISTRIBUTION_LABOR_PER_UNIT;
    mean_consumption_frequency = product->mean_consumption_frequency;
}

