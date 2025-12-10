#pragma once

#include <vector>

#include "Accountant.hpp"
#include "Distributor.hpp"
#include "Firm.hpp"
#include "Good.hpp"
#include "Worker.hpp"

struct Config {
        double plan_cycle_duration = 90;
        // Fraction of income that is consumed (FIC)
        double fic = 0.8;
        double workday_length = 8.0;
};

class Society {
    private:
        void tick();

    public:
        std::vector<Worker *> workers;
        std::vector<Firm *> firms;
        std::vector<Good *> goods;
        std::vector<Distributor *> distributors;

        Config config;

        Accountant * accountant;

        int plan_cycle = 0;

        double reserve = 0.0;

        Society(Config config);

        void add_worker(Worker * w);
        void add_firm(Firm * f);
        void add_good(Good * g);

        /* For worker index/total, add them to a firm (forced) distributed by firm total workers
         * needed */
        void distribute_worker(Worker * worker, int index, int total);

        void pay(Worker * worker, double amount);

        void tick_cycle();

        AggregateGoodStats aggregate_good_stats(Good *);
};