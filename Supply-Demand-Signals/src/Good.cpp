#include "Good.hpp"

Good::Good(std::string name, double target_surplus, robin_hood::unordered_map<Good *, double> means,
    double labor_required, bool discrete)
    : name(std::move(name)),
      target_surplus(target_surplus),
      means(std::move(means)),
      labor_required(labor_required),
      discrete(discrete) {
    recompute_value();
}

double Good::means_value() {
    double total = 0.0;
    for (const auto & [resource, amount] : means) {
        total += resource->value * amount;
    }
    return total;
}

void Good::recompute_value() { value = labor_required + means_value(); }
