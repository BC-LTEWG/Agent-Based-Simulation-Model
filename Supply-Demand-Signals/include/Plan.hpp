#pragma once

#include "Good.hpp"

struct Plan {
        double fixed_capital;
        double means;
        double labor;

        Good * good;
        double quantity;
        double for_surplus;

        static Plan from(Good * good, double raw_quantity, double for_surplus);
};
