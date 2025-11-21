#pragma once
#ifndef PRICE_CONTROLLER_H
#define PRICE_CONTROLLER_H

#include "Constants.h"
#include "Firm.h"
#include "Product.h"

#include <unordered_map>

#endif


static class PriceController {
    public:
        void update_prices();
        void get_price(Product * product);
        void set_price(Product * product, double price);
        void get_all_prices();
        void set_all_prices(std::unordered_map<Product *, double> prices);
        void get_all_prices();
        void update_plan(Plan * plan);
        void get_plan_history();
    private:
        std::unordered_map<Product *, double> price_history;
        std::vector<Plan *> plan_history;

};