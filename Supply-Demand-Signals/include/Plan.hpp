#pragma once

#include "Good.hpp"

struct Plan {
        double means;
        double resources;
        double labor;

        Good * good;
        double quantity;

        double product;
};
