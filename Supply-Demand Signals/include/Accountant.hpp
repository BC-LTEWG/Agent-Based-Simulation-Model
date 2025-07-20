#pragma once

#include "Plan.hpp"
#include "Project.hpp"
class Accountant {
    private:
    public:
        Accountant();

        bool approve_plan(Plan p, Project previous);
};
