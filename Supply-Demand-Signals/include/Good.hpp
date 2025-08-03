#pragma once

#include <string>

struct Good {
        std::string name;
        double value;
        double target_surplus;

        // If true, only rounded values can be exchanged/created
        bool discrete;
};
