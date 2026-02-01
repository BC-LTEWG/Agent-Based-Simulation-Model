#include "Constants.h"
#include "Logger.h"
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

bool Sim::is_writing_data() {
    return write_data;
}

void Sim::set_write_data(bool should_write) {
    write_data = should_write; 
}

int Sim::get_current_time_step() {
	return current_time_step;
}

void Sim::run(int time_steps) {
    for (std::size_t i = 0; i < time_steps; ++i) {
        society->on_time_step();
        current_time_step++;
    }
    if (write_data) {
        Logger::get_instance()->write_data();
    }
}

int Sim::current_time_step = 0;

bool Sim::trace_logging = false;

bool Sim::write_data = false;

std::random_device Sim::rd;

std::mt19937 Sim::gen(Sim::rd());

