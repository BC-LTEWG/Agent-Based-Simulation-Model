#pragma once

#include <vector>

#include "Plan.hpp"

class Society;

// Forward declaration instead of including Worker.hpp, Firm.hpp
class Worker;
class Firm;

class Project {
        friend class Firm;

    private:
        Society * society;
        Firm * firm;

        std::vector<Worker *> workers;

        double hours_left;

        
        public:
        int plan_cycle;
        
        Plan plan;
        int ideal_workers;
        
        double goods_produced = 0.0;

        Project(Society * society, Plan plan);

        int num_workers();

        void add_worker(Worker * w);

        void tick();
};
