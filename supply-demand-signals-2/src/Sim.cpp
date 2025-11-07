#include "Sim.h"

std::random_device Sim::rd;

std::mt19937 Sim::gen(Sim::rd());

Sim::Sim(Society* society) : current_time_step(0), society(society) {}

