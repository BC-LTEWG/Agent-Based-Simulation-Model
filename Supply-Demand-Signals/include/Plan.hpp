#pragma once

#include "Good.hpp"

struct Plan {
        double fixed_capital;
        double means;
        double labor;

        Good * good;
        double quantity;

        double product;

        static Plan from(Good * good, double quantity);
};
