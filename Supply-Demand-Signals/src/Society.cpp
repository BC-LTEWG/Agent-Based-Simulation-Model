#include "../include/Society.hpp"

#include <algorithm>
#include <cstdio>
#include <vector>

#include "Good.hpp"

Society::Society(Config config) : config(config), accountant(new Accountant()) {}

void Society::add_worker(Worker * w) { workers.push_back(w); }

void Society::add_firm(Firm * f) { firms.push_back(f); }

void Society::add_good(Good * g) { goods.push_back(g); }

void Society::distribute_worker(Worker * worker, int index, int total) {
    double frac = (index + 1) / static_cast<double>(total);

    int total_jobs = 0;
    for (auto & firm : firms) {
        total_jobs += firm->total_ideal_jobs();
    }

    double running = 0;
    int firm_idx = 0;
    while (running < frac) {
        double firm_share = firms[firm_idx]->total_ideal_jobs() / static_cast<double>(total_jobs);
        running += firm_share;
        firm_idx++;
    }

    firms[firm_idx - 1]->employ(worker, true);
}

void Society::pay(Worker * worker, double amount) {
    double earned = amount * config.fic;
    worker->pay(earned);
    reserve += amount * (1 - config.fic);
}

void Society::tick_cycle() {
    // Go through the cycle duration in daily ticks
    for (int day = 0; day < config.plan_cycle_duration; day++) {
        tick();
    }

    plan_cycle++;

    // Develop new plans for next cycle:
    // Beginning of plan cycle - generate plans
    for (auto & firm : firms) {
        firm->new_plans();
    }

    // Step 2: Distribute workers to firms

    // Build a list of projects
    std::vector<Project *> all_projects;
    for (auto & firm : firms) {
        for (auto & project : firm->all_projects()) {
            all_projects.push_back(project);
        }
    }

    // Sort projects by how understaffed they are, by ratio
    std::sort(all_projects.begin(), all_projects.end(), [](Project * a, Project * b) {
        double ratio_a = static_cast<double>(a->num_workers()) / a->ideal_workers;
        double ratio_b = static_cast<double>(b->num_workers()) / b->ideal_workers;
        return ratio_a < ratio_b;
    });

    // Assign workers to projects
    for (auto & worker : workers) {
        if (worker->employed) {
            continue;
        }
        for (auto & project : all_projects) {
            if (project->num_workers() < project->ideal_workers) {
                project->add_worker(worker);
                worker->employed = true;
                break;
            }
        }
    }
}

void Society::tick() {
    for (auto & firm : firms) {
        firm->tick();
    }

    for (auto & worker : workers) {
        if (!worker->employed) {
            const double payable = std::min(config.workday_length * config.fic, reserve);
            worker->pay(payable);
            reserve -= payable;
        }
        worker->update_needs();
        worker->fulfill_needs();
    }
}

AggregateGoodStats Society::aggregate_good_stats(Good * good) {
    std::vector<Project *> all_projects;
    for (auto & firm : firms) {
        for (auto & project : firm->all_projects()) {
            if (project->plan.good == good) {
                all_projects.push_back(project);
            }
        }
    }

    AggregateGoodStats stats = {0.0, 0.0, 0.0};
    for (auto & project : all_projects) {
        stats.planned_quantity += project->plan.quantity;
        stats.produced += project->goods_produced;
        stats.ideal_workers += project->ideal_workers;
    }
    return stats;
}