#pragma once

#include "Good.hpp"

struct Plan {
        double fixed_capital;
        double means;
        double labor;

        Good * good;
        double quantity;

        static Plan from(Good * good, double quantity);
};
