#pragma once

#include <vector>

#include "Plan.hpp"

class Society;

// Forward declaration instead of including Worker.hpp
class Worker;

class Project {
        friend class Firm;

    private:
        Society * society;

        std::vector<Worker *> workers;

        double hours_left;

        double goods_produced = 0.0;

    public:
        int plan_cycle;

        Plan plan;
        int ideal_workers;

        Project(Society * society, Plan plan);

        int num_workers();

        void add_worker(Worker * w);

        void tick();
};
