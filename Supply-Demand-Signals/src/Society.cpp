#include "../include/Society.hpp"

#include <algorithm>
#include <cstdio>

Society::Society(Config config) : config(config), accountant(new Accountant()) {}

void Society::add_worker(Worker * w) { workers.push_back(w); }

void Society::add_firm(Firm * f) { firms.push_back(f); }

void Society::add_good(Good * g) { goods.push_back(g); }

void Society::pay(Worker * worker, double amount) {
    double earned = amount * config.fic;
    worker->pay(earned);
    reserve += amount * (1 - config.fic);
}

void Society::tick_cycle(bool is_first) {
    if (!is_first) {
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

    // Go through the cycle duration in daily ticks
    for (int day = 0; day < config.plan_cycle_duration; day++) {
        tick();
    }

    plan_cycle++;
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