#include "../include/Firm.hpp"
#include <cstdio>
#include <vector>

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
    // std::vector<Project *> new_projects;
    // for (auto & project : projects) {
        
    // }

    // projects = std::move(new_projects);
}