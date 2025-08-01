#include "../include/Firm.hpp"

#include <cstdio>
#include <stdexcept>
#include <vector>

#include "../include/Society.hpp"

Firm::Firm(Society * society) : society(society) {}

bool Firm::should_hire() {
    for (auto & project : projects) {
        if (project->num_workers() < project->ideal_workers) {
            return true;
        }
    }
    return false;
}

double Firm::job_availability() {
    int total_ideal = 0;
    int total_current = 0;
    for (auto & project : projects) {
        total_ideal += project->ideal_workers;
        total_current += project->num_workers();
    }
    if (total_ideal == 0) {
        return 0.0;
    }
    return static_cast<double>(total_ideal - total_current) / total_ideal;
}

int Firm::total_ideal_jobs() {
    int total_ideal = 0;
    for (auto & project : projects) {
        total_ideal += project->ideal_workers;
    }
    return total_ideal;
}

void Firm::add_project(Project * p) { projects.push_back(p); }

std::vector<Project *> Firm::all_projects() { return projects; }

Project * Firm::least_staffed_project() {
    Project * least = nullptr;
    double least_ratio = 1.0;
    for (auto & project : projects) {
        double ratio = static_cast<double>(project->num_workers()) / project->ideal_workers;
        if (ratio < least_ratio) {
            least_ratio = ratio;
            least = project;
        }
    }
    return least;
}

bool Firm::employ(Worker * w) {
    // find best project to assign worker to
    Project * best_project = nullptr;

    best_project = least_staffed_project();

    if (best_project) {
        best_project->add_worker(w);
        return true;
    }

    return false;
}

void Firm::add_stock(Good * good, Project * project, double amount) {
    if (!inventory.count(good)) {
        inventory[good] =
            InventoryItem{robin_hood::unordered_map<Project *, double>(), std::vector<double>()};
        // Fill deficit history with zeros up to society plan cycle
        inventory[good].deficit_history.resize(society->plan_cycle + 1, 0.0);
    }

    if (!inventory[good].projects.count(project)) {
        inventory[good].projects[project] = 0.0;
    }

    inventory[good].projects[project] += amount;
}

double Firm::total_inventory(Good * good) {
    if (!inventory.count(good)) {
        return 0.0;
    }

    double total = 0.0;
    for (const auto & [project, amount] : inventory[good].projects) {
        total += amount;
    }
    return total;
}

void Firm::tick() {
    for (auto & project : projects) {
        project->tick();
    }
}

void Firm::new_plans() {
    for (auto & good_pair : inventory) {
        // Shift deficit history to the right
        auto & deficit_history = good_pair.second.deficit_history;
        while (deficit_history.size() <= (unsigned long int)society->plan_cycle) {
            deficit_history.push_back(0.0);
        }
    }

    std::vector<Project *> new_projects;

    // Pool of workers not currently assigned to a project
    std::vector<Worker *> unassigned_workers;

    for (auto & project : projects) {
        printf("Project with %d/%d workers completing plan of %s: %.2f hours left\n",
            project->num_workers(),
            project->ideal_workers,
            project->plan.good->name.c_str(),
            project->hours_left);
        auto new_plan = generate_plan(project);

        while (!society->accountant->approve_plan(new_plan, *project)) {
            // TODO: generate a new plan / change plan parameters
            throw std::runtime_error("Plan was not approved by accountant");
        }

        // Create the new project and assign as many workers as possible
        auto new_project = new Project(society, new_plan);
        while (
            new_project->num_workers() < new_project->ideal_workers && project->num_workers() > 0) {
            // transfer workers to new project
            auto worker = project->workers.back();
            project->workers.pop_back();
            new_project->add_worker(worker);
        }

        // Assign unassigned workers to the new project
        while (new_project->num_workers() < new_project->ideal_workers &&
               !unassigned_workers.empty()) {
            auto worker = unassigned_workers.back();
            unassigned_workers.pop_back();
            new_project->add_worker(worker);
        }

        new_projects.push_back(new_project);

        // Add remaining workers to unassigned pool
        unassigned_workers.insert(unassigned_workers.end(),
            project->workers.begin(),
            project->workers.end());
    }

    // tell all the unassigned workers they are unemployed
    for (auto & worker : unassigned_workers) {
        worker->employed = false;
    }

    // Replace old projects with new projects
    projects = std::move(new_projects);
}

Plan Firm::generate_plan(Project * project) {
    if (project->plan_cycle < 2) {
        // for the first two cycles, we don't have enough data to make a new plan
        // so just return the existing plan
        return project->plan;
    }

    // Step 1: estimate the acutal necessity of the good
    double remaining_inventory = society->distributors[0]->get_project_inventory(project);
    double deficit =
        society->distributors[0]->get_production_deficit(project->plan.good, project->plan_cycle);

    // How much people actually consume
    double consumption = project->goods_produced - remaining_inventory + deficit;

    printf("Estimated consumption for %s: %.2f (remaining: %.2f, deficit: %.2f)\n",
        project->plan.good->name.c_str(),
        consumption,
        remaining_inventory,
        deficit);

    double estimated_necessity = consumption - remaining_inventory;

    estimated_necessity = std::max(estimated_necessity, 0.0);

    // Step 2: adjust the plan quantity based on estimated necessity
    double multiplier =
        project->goods_produced > 0 ? estimated_necessity / project->goods_produced : 0;

    printf("Produced: %.2f, Estimated necessity: %.2f\n, Multiplier: %.2f\n",
        project->goods_produced,
        estimated_necessity,
        multiplier);

    Plan new_plan = {.means = project->plan.means * multiplier,
        .resources = project->plan.resources * multiplier,
        .labor = project->plan.labor * multiplier,

        .good = project->plan.good,
        .quantity = project->plan.quantity * multiplier,

        .product = project->goods_produced * multiplier};

    return new_plan;
}