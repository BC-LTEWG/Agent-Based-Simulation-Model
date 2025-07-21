#include "../include/Society.hpp"

#include <cstdio>

Society::Society(int plan_cycle_duration)
    : plan_cycle_duration(plan_cycle_duration), accountant(new Accountant()) {}

void Society::add_worker(Worker * w) { workers.push_back(w); }

void Society::add_firm(Firm * f) { firms.push_back(f); }

void Society::add_good(Good * g) { goods.push_back(g); }

void Society::pay(Worker * worker, double amount) {
    double taxed_amount = amount * (1.0 - tax_rate);
    worker->pay(taxed_amount);
    reserve += amount * tax_rate;
}

void Society::tick_cycle(bool is_first) {
    if (!is_first) {
        // Beginning of plan cycle - generate plans
        for (auto & firm : firms) {
            firm->new_plans();
        }
    }

    // Go through the cycle duration in daily ticks
    for (int day = 0; day < plan_cycle_duration; day++) {
        tick();
    }

    plan_cycle++;
}

void Society::tick() {
    for (auto & firm : firms) {
        firm->tick();
    }

    for (auto & worker : workers) {
        worker->update_needs();
        worker->fulfill_needs();
    }
}