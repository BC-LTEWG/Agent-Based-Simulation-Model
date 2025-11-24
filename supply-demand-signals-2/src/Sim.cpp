#include "Constants.h"
#include "Sim.h"

std::random_device Sim::rd;

std::mt19937 Sim::gen(Sim::rd());

Sim::Sim() {
	society = new Society();
}

int Sim::get_current_time_step() {
	return current_time_step;
}

int Sim::current_time_step = 0;

int main() {
	Sim simulation;
	return EXIT_SUCCESS;
}
