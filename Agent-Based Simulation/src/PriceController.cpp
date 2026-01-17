#include <iostream>

#include "Constants.h"
#include "Distributor.h"
#include "Firm.h"
#include "PriceController.h"
#include "Product.h"

void PriceController::update_price(Plan * plan) {
    Product * product = plan->order->product;
    double actual_labor = plan->labor_hours - plan->labor_hours_remaining;
    double new_average_labor =
        (actual_labor +
         product->living_labor_per_unit * (PRICE_AVERAGING_WINDOW - 1)) /
        PRICE_AVERAGING_WINDOW;
    double old_average_labor = product->living_labor_per_unit;
    product->living_labor_per_unit = new_average_labor;
    product->price_per_unit =
        product->price_per_unit - old_average_labor + new_average_labor;
}

