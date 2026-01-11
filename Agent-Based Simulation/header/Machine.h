#pragma once

#include <string>
#include <vector>

#include "Product.h"

struct Machine : public Product {
    Machine(const std::string& name, int lifetime, double cost_of_machine);
    const int lifetime;
    int hours_remaining = 0;
    double cost_of_machine;
};
