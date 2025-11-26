#include "Constants.h"
#include "Sim.h"

std::random_device Sim::rd;

std::mt19937 Sim::gen(Sim::rd());

Sim::Sim() {
	society = new Society();
}
<<<<<<< HEAD
=======

int Sim::get_current_time_step() {
	return current_time_step;
}

int Sim::current_time_step = 0;

int Sim::get_current_time_step() {
	return current_time_step;
}

int Sim::current_time_step = 0;

int main() {
	Sim simulation;
	return EXIT_SUCCESS;
}
>>>>>>> 1216f2094b18f1f7c0ad89c94cbb1d9b5b03f344
