#pragma once

#include <string>

#include "../extern/robin_hood/robin_hood.h"

struct Good {
        std::string name;
        double target_surplus;

        robin_hood::unordered_map<Good *, double> means;
        double labor_required;

        // If true, only rounded values can be exchanged/created
        bool discrete;

        double value;

        Good(std::string name, double target_surplus = 0.0,
            robin_hood::unordered_map<Good*, double> means = {}, double labor_required = 0.0,
            bool discrete = false);

        double means_value();

        /**
         * For performance reasons, value is fixed
         * and will only change when manually recomputed
         */
        void recompute_value();
};
