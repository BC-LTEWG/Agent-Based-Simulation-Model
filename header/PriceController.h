#pragma once

#include <unordered_map>
#include <vector>

struct Plan;
struct Product;

class PriceController {
    public:
        static PriceController * get_instance();
        void update_price(Plan *);
    private:
        PriceController();
        std::unordered_map<Product *, std::vector<std::pair<Plan *, int>>> plan_history;
};

