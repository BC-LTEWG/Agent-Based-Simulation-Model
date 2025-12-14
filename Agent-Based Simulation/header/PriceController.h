#pragma once
#ifndef PRICE_CONTROLLER_H
#define PRICE_CONTROLLER_H

#include <iostream>
#include <unordered_map>
#include <vector>

#include "Agent.h"
#include "Machine.h"
#include "Firm.h"
#include "Constants.h"

struct Plan;
struct Machine;
struct Product;

class PriceController : public Agent {
    public:
        void on_time_step() override;
        static void update_prices();
        static double get_price(Product * product);

        static void set_price(Product * product, double price);
        static double get_price(Product * product, Firm * firm);
        static void set_price(Product * product, Firm * firm, double price);

        static void get_all_prices();
        static void set_all_prices(std::unordered_map<Product *, double> prices);
        static void update_plan(Plan * plan);
        static void get_plan_history();
        static double machine_price_computation(Machine * machine);
        static double product_price(Product * product, Firm * firm);
        static void poll_prices();
    private:
        static inline std::unordered_map<Product *, double> price_history;
        static inline std::unordered_map<Firm *, std::unordered_map<Product *, double>> firm_price_history;
        static inline std::vector<Plan *> plan_history;
};

#endif
