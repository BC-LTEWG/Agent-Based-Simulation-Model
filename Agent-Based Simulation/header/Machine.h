#pragma once

#include <string>
#include <vector>

#include "Product.h"

struct Machine : public Product {
    Machine(const std::string& name, int lifetime);
    const int lifetime;
    int hours_used = 0;
};
