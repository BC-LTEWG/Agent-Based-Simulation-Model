#pragma once

#include "Plan.hpp"
class Accountant {
    private:
    public:
        Accountant();

        bool approve_plan(Plan p, Plan previous);
};
