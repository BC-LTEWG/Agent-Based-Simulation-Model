#include "Constants.h"
#include "Sim.h"

Sim::Sim() {
	society = Society::get_instance();
}

bool Sim::is_trace_logging() {
    return trace_logging;
}

void Sim::set_trace_logging(bool should_trace) {
    trace_logging = should_trace; 
}

int Sim::get_current_time_step() {
	return current_time_step;
}

void Sim::run(int time_steps) {
    for (std::size_t i = 0; i < time_steps; ++i) {
        society->on_time_step();
        current_time_step++;
    }
}

int Sim::current_time_step = 0;

bool Sim::trace_logging = false;

std::random_device Sim::rd;

std::mt19937 Sim::gen(Sim::rd());

