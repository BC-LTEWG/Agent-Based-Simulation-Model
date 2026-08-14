#pragma once

#include <unordered_map>
#include "Order.h"
#include "Firm.h"
#include "Product.h"

struct Plan {
    inline static int next_id = 0;
    const int id = next_id++;

    Order * order;
    Firm * firm;
    std::vector<Person *> workers;
    unsigned int local_work_hours_daily = 0;
    double machinery_budget = 0.0;
    double labor_budget = 0.0;
    double raw_materials_budget = 0.0;
    double debt = 0.0;
    double labor_hours_used = 0.0;
    std::unordered_map<Product *, double> inventory;
    std::unordered_map<Product *, double> outlays;
    double quantity_remaining = 0.0;
    bool is_stalled = false;
    Product * missing_resource = nullptr;
};

