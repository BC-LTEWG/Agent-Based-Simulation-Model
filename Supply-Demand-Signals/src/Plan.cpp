#include "Plan.hpp"
Plan Plan::from(Good * good, double quantity) {
    return Plan{
        .means = 0,
        .resources = 0,
        .labor = good->value * quantity,

        .good = good,

        .quantity = quantity,
        .product = 0 + 0 + good->value * quantity,
    };
}