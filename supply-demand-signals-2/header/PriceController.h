#pragma once
#ifndef PRICE_CONTROLLER_H
#define PRICE_CONTROLLER_H

#include "Agent.h"
#include "Constants.h"
#include "Firm.h"
#include "Product.h"

#include <iostream>
#include <unordered_map>
#include <vector>

struct Plan;

class PriceController : public Agent {
    public:
        void on_time_step() override;
        static void update_prices();
        static void get_price(Product * product);
        static void set_price(Product * product, double price);
        static void get_all_prices();
        static void set_all_prices(std::unordered_map<Product *, double> prices);
        static void update_plan(Plan * plan);
        static void get_plan_history();
    private:
        static inline std::unordered_map<Product *, double> price_history;
        static inline std::vector<Plan *> plan_history;
};

#endif
