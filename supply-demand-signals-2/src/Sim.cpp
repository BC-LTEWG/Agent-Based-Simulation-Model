#include "Sim.h"

Sim::Sim() : time_step(0) {}

void Sim::advance_time_step() {
}

void Sim::run_sim(int days) {
}

void Sim::run_work_day() {
}

void Sim::reset_time_step() {
    time_step = std::chrono::hours(0);
}

