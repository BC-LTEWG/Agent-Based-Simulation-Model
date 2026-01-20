#include "Constants.h"
#include "Sim.h"

Sim::Sim() {
	society = Society::get_instance();
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

std::random_device Sim::rd;

std::mt19937 Sim::gen(Sim::rd());

