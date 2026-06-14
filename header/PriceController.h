#pragma once

#include <unordered_map>
#include <vector>

#include "Agent.h"

struct Plan;
struct Product;

class PriceController : public Agent {
    public:
        static PriceController * get_instance();
        void on_time_step() override;
        unsigned int get_id() override; 
        void update_price(Plan *);
        void report_distribution(ConsumerGood *, int quantity);
        double get_fic();
    private:
        PriceController();
        double fic = 1.0;
        std::unordered_map<Product *, std::vector<std::pair<Plan *, int>>> plan_history;
        std::unordered_map<ConsumerGood *, double> total_consumer_good_distribution_value;
        void update_fic();
        void log_public_sector_distribution_value(double);
        void log_all_distribution_value(double);
        void log_fic();
};

