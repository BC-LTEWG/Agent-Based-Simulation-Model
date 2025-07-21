#include "../include/Project.hpp"

#include "../include/Society.hpp"

Project::Project(Society * society, Plan plan)
    : society(society),
      plan(plan),
      hours_left(plan.means + plan.labor + plan.resources),
      ideal_workers(plan.labor / society->workday_length / society->plan_cycle_duration) {
    plan_cycle = society->plan_cycle;
}

void Project::add_worker(Worker * w) { workers.push_back(w); }

int Project::num_workers() { return workers.size(); }

void Project::tick() {
    double workday_length = society->workday_length;
    if (hours_left <= 0 || workers.empty()) {
        // we have nothing left to spend
        return;
    }

    // assume we do 1/plan_cycle_duration of the work each day
    double hours_worked = std::min(workday_length, hours_left / workers.size());
    if (hours_worked <= 0) {
        // we have nothing left to spend
        return;
    }

    hours_left -= hours_worked * num_workers();

    for (auto & worker : workers) {
        society->pay(worker, hours_worked);
    }

    if (!society->distributors.empty()) {
        double fraction = hours_worked / workday_length;
        double per_dist =
            plan.quantity / society->plan_cycle_duration / fraction / society->distributors.size();
        for (auto & distributor : society->distributors) {
            distributor->add_stock(plan.good, this, per_dist);
        }
    }
}