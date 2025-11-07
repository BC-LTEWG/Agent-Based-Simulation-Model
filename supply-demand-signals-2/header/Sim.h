#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "Society.h"

class Sim {
public:
    Sim();
    void advance_time_step();
    void run_sim(int days);
    void run_work_day();
    void reset_time_step();

private:
    std::chrono::hours time_step;
};
