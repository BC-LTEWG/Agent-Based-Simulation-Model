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

void Firm::tick() {
    for (auto & project : projects) {
        project->tick();
    }
}

void Firm::new_plans() {
    std::vector<Project *> new_projects;

    // Pool of workers not currently assigned to a project
    std::vector<Worker *> unassigned_workers;

    for (auto & project : projects) {
        printf("Project with %d/%d workers completing plan of %s: %.2f hours left\n",
            project->num_workers(), project->ideal_workers, project->plan.good->name.c_str(),
            project->hours_left);
        auto new_plan = generate_plan(project);

        while (!society->accountant->approve_plan(new_plan, *project)) {
            // TODO: generate a new plan / change plan parameters
            throw std::runtime_error("Plan was not approved by accountant");
        }

        // Create the new project and assign as many workers as possible
        auto new_project = new Project(society, new_plan);
        while (new_project->num_workers() < new_project->ideal_workers && project->num_workers() > 0) {
            // transfer workers to new project
            auto worker = project->workers.back();
            project->workers.pop_back();
            new_project->add_worker(worker);
        }

        // Assign unassigned workers to the new project
        while (new_project->num_workers() < new_project->ideal_workers && !unassigned_workers.empty()) {
            auto worker = unassigned_workers.back();
            unassigned_workers.pop_back();
            new_project->add_worker(worker);
        }

        new_projects.push_back(new_project);

        // Add remaining workers to unassigned pool
        unassigned_workers.insert(unassigned_workers.end(), project->workers.begin(), project->workers.end());
    }

    // Replace old projects with new projects
    projects = std::move(new_projects);
}

Plan Firm::generate_plan(Project * project) {
    // current plan for now
    return project->plan;
}