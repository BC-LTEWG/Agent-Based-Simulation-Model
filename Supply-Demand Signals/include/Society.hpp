#pragma once

#include <vector>

#include "Accountant.hpp"
#include "Distributor.hpp"
#include "Firm.hpp"
#include "Good.hpp"
#include "Worker.hpp"

struct Config {
    double plan_cycle_duration = 90;
    double fic = 0.8;
    double workday_length = 8.0;
};

class Society {
    private:
        void tick();

        double reserve = 0.0;

    public:
        std::vector<Worker *> workers;
        std::vector<Firm *> firms;
        std::vector<Good *> goods;
        std::vector<Distributor *> distributors;

        Config config;

        Accountant * accountant;

        int plan_cycle = 0;

        Society(Config config);

        void add_worker(Worker * w);
        void add_firm(Firm * f);
        void add_good(Good * g);

        void pay(Worker * worker, double amount);

        void tick_cycle(bool is_first = false);
};
