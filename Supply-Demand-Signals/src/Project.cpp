#include "Project.hpp"
#include "Firm.hpp"

#include <cmath>

#include "../include/Society.hpp"

Project::Project(Society * society, Plan plan)
    : society(society),
      plan_cycle(society->plan_cycle),
      plan(plan),
      ideal_workers(std::ceil(
          plan.labor / society->config.workday_length / society->config.plan_cycle_duration)) {
    hours_left = plan.means + plan.labor + plan.fixed_capital;
}

void Project::add_worker(Worker * w) { workers.push_back(w); }

int Project::num_workers() { return workers.size(); }

void Project::tick() {
    double workday_length = society->config.workday_length;
    if (hours_left <= 0 || workers.empty()) {
        // we have nothing left to spend
        return;
    }

    double production_cost_per = plan.good->value;

    double amount_to_produce =
        std::min(hours_left, workday_length * std::min(ideal_workers, num_workers())) /
        production_cost_per;

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

        max_production_fraction =
            std::min(max_production_fraction, available / total_needed);
    }

    amount_to_produce *= max_production_fraction;

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

    // produce goods proportional to labor done against total required labor
    double produced = (labor_done / plan.labor) * plan.quantity;
    goods_produced += produced;

    double per_dist = produced / society->distributors.size();

    for (auto & distributor : society->distributors) {
        distributor->add_stock(plan.good, this, per_dist);
    }
}