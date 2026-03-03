#pragma once

#include <random>
#include <string>
#include <vector>

#include "Constants.h"
#include "Society.h"

struct SimArgs {
    unsigned int time_steps = NUM_SIM_RUNS;
    bool csv = false;
    bool db = false;
    bool json = false;
};

class Sim {
    public:
        static Sim& get_instance();
        static void run(SimArgs& args);
        static bool does_csv();
        static bool does_db();
        static bool does_json();
        static unsigned int get_current_time_step();
        static std::random_device& get_random_device();
        static std::mt19937& get_random_generator();
        void set_params(SimArgs& args);
    private:
        Sim();
        void run();
        SimArgs args;
        std::random_device rd;
        std::mt19937 gen;
        unsigned int current_time_step;
        Society * society;
};
