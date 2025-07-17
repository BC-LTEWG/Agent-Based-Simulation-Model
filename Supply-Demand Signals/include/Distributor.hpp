#pragma once

#include <utility>
#include "Firm.hpp"

class Distributor : public Firm {
    private:
    public:
        Distributor(Society * society);

    void add_stock(Good * good, double amount);

    // Returns <quantity purchased, credits spent>
    std::pair<double, double> purchase(Good * good, double amount);

    // 0 rows = all
    void display_inventory(int rows = 0);

    // display first few rows of inventory
    void head();
};
