#pragma once

#include <vector>

#include "Distributor.hpp"
#include "Good.hpp"

struct WorkerNeed {
        Good * good;
        double amount;

        // lower number means higher priority
        double priority;
};

struct WorkerNeedCycle {
    WorkerNeed * need;
    // the number of ticks until the worker needs this again
    int clock;
    // the number of ticks it takes for this need to replenish
    int cycles;
};

class Worker {
    private:
        Distributor * distributor;
        double credits = 0.0;

        // The goods that the worker needs, as a priority queue
        // Workers will fulfill their needs as they get credits
        std::vector<WorkerNeed *> needs;

        std::vector<WorkerNeedCycle> need_cycles;

        std::vector<WorkerNeed*> cycle_needs();
    public:

        bool employed = false;

        Worker(Distributor * distributor, std::vector<WorkerNeedCycle> need_cycles, double initial_credits);

        double wealth();

        void pay(double amount);
        // Returns whether or not the worker had enough credits to spend
        bool spend(double amount);

        int need_count();
        
        void update_needs();
        // Fulfill needs in order of priority
        void fulfill_needs();

        void change_needs(std::vector<WorkerNeedCycle> new_need_cycles);
};
