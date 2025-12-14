#include "Constants.h"
#include "Sim.h"

Sim::Sim() {
	society = new Society();
}

int Sim::get_current_time_step() {
	return current_time_step;
}

int Sim::current_time_step = 0;

std::random_device Sim::rd;

std::mt19937 Sim::gen(Sim::rd());

