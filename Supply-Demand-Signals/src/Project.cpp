#include "Project.hpp"

#include <cmath>
#include <cstdio>

#include "../include/Society.hpp"
#include "Firm.hpp"

Project::Project(Society * society, Plan plan)
    : society(society),
      hours_left(plan.means + plan.labor + plan.fixed_capital),
      surplus_delta(society->distributors[0]->total_inventory(plan.good)),
      plan_cycle(society->plan_cycle),
      plan(plan),
      ideal_workers(std::ceil(
          plan.labor / society->config.workday_length / society->config.plan_cycle_duration)) {}

void Project::add_worker(Worker * w) { workers.push_back(w); }

int Project::num_workers() { return workers.size(); }

void Project::tick() {
    double workday_length = society->config.workday_length;
    if (hours_left <= 0 || workers.empty()) {
        // we have nothing left to spend
        return;
    }

    double production_cost_per = plan.good->value;

    double amount_to_produce = std::min(hours_left / production_cost_per,
        workday_length * std::min(ideal_workers, num_workers()) / plan.good->labor_required);

    if (amount_to_produce <= 0) return;

    double max_production_fraction = 1;
    for (auto [good, quantity] : plan.good->means) {
        double total_needed = quantity * amount_to_produce;
        double available = firm->total_inventory(good);
        double required = std::max(0.0, total_needed - available);
        if (required > 0) {
            auto [purchased, cost] = society->distributors[0]->purchase(good, required);
            hours_left -= cost;
            firm->add_stock(good, nullptr, purchased);

            available += purchased;
        }

        max_production_fraction = std::min(max_production_fraction, available / total_needed);
    }

    amount_to_produce *= max_production_fraction;

    // printf("[%s] producing %.2f/%.2f\n",
    //     plan.good->name.c_str(),
    //     amount_to_produce,
    //     plan.quantity / society->config.plan_cycle_duration);

    // extract from inventory the means we need
    for (auto [good, quantity] : plan.good->means) {
        firm->take_from_inventory(good, quantity * amount_to_produce);
    }

    double hours_worked = amount_to_produce * plan.good->labor_required / num_workers();

    // subtract actual labor performed
    double labor_done = hours_worked * num_workers();
    hours_left -= labor_done;

    for (auto & worker : workers) {
        society->pay(worker, workday_length);
    }

    goods_produced += amount_to_produce;

    double per_dist = amount_to_produce / society->distributors.size();

    for (auto & distributor : society->distributors) {
        distributor->add_stock(plan.good, this, per_dist);
    }
}