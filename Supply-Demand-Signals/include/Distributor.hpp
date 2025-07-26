#pragma once

#include <utility>
#include "Firm.hpp"

class Distributor : public Firm {
    private:
    public:
        Distributor(Society * society);

    // Returns <quantity purchased, credits spent>
    std::pair<double, double> purchase(Good * good, double amount);

    // Get the remaining inventory for a given project
    double get_project_inventory(Project * project);

    double get_production_deficit(Good * good, int plan_cycle);

    // 0 rows = all
    void display_inventory(int rows = 0);

    // display first few rows of inventory
    void head();
};
