#pragma once

#include <string>
#include <vector>

#include "Product.h"

struct Machine : public Product {
    Machine();
    int lifetime;
};
