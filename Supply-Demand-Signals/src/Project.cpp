#include "../include/Project.hpp"

#include <cmath>

#include "../include/Society.hpp"

Project::Project(Society * society, Plan plan)
    : society(society),
      plan_cycle(society->plan_cycle),
      plan(plan),
      ideal_workers(std::ceil(
          plan.labor / society->config.workday_length / society->config.plan_cycle_duration)) {
    hours_left = plan.means + plan.labor + plan.resources;
}

void Project::add_worker(Worker * w) { workers.push_back(w); }

int Project::num_workers() { return workers.size(); }

void Project::tick() {
    double workday_length = society->config.workday_length;
    if (hours_left <= 0 || workers.empty()) {
        // we have nothing left to spend
        return;
    }

    // assume we do 1/plan_cycle_duration of the work each day
    double hours_worked = std::min(workday_length, hours_left / num_workers());
    if (hours_worked <= 0) {
        // we have nothing left to spend
        return;
    }

    hours_left -= hours_worked * num_workers();

    for (auto & worker : workers) {
        society->pay(worker, hours_worked);
    }

    double fraction_of_workday = hours_worked / workday_length;

    double produced = plan.quantity / society->config.plan_cycle_duration * fraction_of_workday;
    goods_produced += produced;

    double per_dist = produced / society->distributors.size();

    for (auto & distributor : society->distributors) {
        distributor->add_stock(plan.good, this, per_dist);
    }
}