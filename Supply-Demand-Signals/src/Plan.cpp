#include "Plan.hpp"
Plan Plan::from(Good * good, double raw_quantity, double for_surplus) {
    double quantity = raw_quantity + for_surplus;
    return Plan{
        .fixed_capital = 0,
        .means = good->means_value() * quantity,
        .labor = good->labor_required * quantity,

        .good = good,

        .quantity = quantity,
        .for_surplus = for_surplus
    };
}