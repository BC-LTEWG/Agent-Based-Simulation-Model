#include "Plan.hpp"
Plan Plan::from(Good * good, double quantity) {
    return Plan{
        .fixed_capital = 0,
        .means = good->means_value(),
        .labor = good->value * quantity,

        .good = good,

        .quantity = quantity,
        .product = 0 + 0 + good->value * quantity,
    };
}