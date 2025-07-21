#pragma once

#include <vector>

#include "Accountant.hpp"
#include "Distributor.hpp"
#include "Firm.hpp"
#include "Good.hpp"
#include "Worker.hpp"

class Society {
    private:
        void tick();

        double reserve = 0.0;

    public:
        std::vector<Worker *> workers;
        std::vector<Firm *> firms;
        std::vector<Good *> goods;
        std::vector<Distributor *> distributors;

        // For now, a fixed plan cycle duration
        int plan_cycle_duration;

        // Standard workday length in hours
        double workday_length = 8.0;

        double tax_rate = 0.2;

        Accountant * accountant;

        int plan_cycle = 0;

        Society(int plan_cycle_duration);

        void add_worker(Worker * w);
        void add_firm(Firm * f);
        void add_good(Good * g);

        void pay(Worker * worker, double amount);

        void tick_cycle(bool is_first = false);
};
