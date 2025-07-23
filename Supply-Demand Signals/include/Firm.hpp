#pragma once

#include <unordered_map>
#include <vector>

#include "Good.hpp"
#include "Project.hpp"

class Society;

struct InventoryItem {
        std::unordered_map<Project *, double> projects;
        std::vector<double> deficit_history;
};

typedef std::unordered_map<Good *, InventoryItem> Inventory;

class Firm {
        friend class Distributor;

    private:
        std::vector<Project *> projects;
        Society * society;
        Inventory inventory;

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

        std::vector<Project *> all_projects();

        void add_project(Project * p);

        // Attempts to employ a worker, returns true if successful
        // false if there are no open positions
        bool employ(Worker * w);

        void add_stock(Good * good, Project * project, double amount);
        double total_inventory(Good * good);

        // Runs one day of production for all projects
        void tick();

        void new_plans();
        Plan generate_plan(Project * project);
};
