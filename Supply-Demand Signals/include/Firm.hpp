#pragma once

#include <unordered_map>
#include <vector>

#include "Project.hpp"

class Society;

class Firm {
    friend class Distributor;

    private:
        std::vector<Project *> projects;
        Society * society;
        std::unordered_map<Good *, double> inventory;

        // Returns the project with the lowest worker-to-ideal-worker ratio.
        // Returns nullptr if there are no projects that have workers < ideal workers.
        Project * least_staffed_project();

    public:
        Firm(Society * society);

        bool should_hire();
        // Returns a value between 0.0 and 1.0 indicating the fraction of ideal jobs
        // that are currently unfilled
        double job_availability();
        int total_ideal_jobs();

        void add_project(Project * p);

        // Attempts to employ a worker, returns true if successful
        // false if there are no open positions
        bool employ(Worker * w);

        // Runs one day of production for all projects
        void tick();

        void new_plans();
};
