#include "Plan.hpp"
Plan Plan::from(Good * good, double quantity) {
    return Plan{
        .fixed_capital = 0,
        .means = good->means_value() * quantity,
        .labor = good->labor_required * quantity,

        .good = good,

        .quantity = quantity,
    };
}