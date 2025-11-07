#pragma once

#include <random>
#include <string>
#include <vector>

#include "Society.h"

class Sim {
  public:
	static std::random_device rd;
	static std::mt19937 gen;

    Sim(Society* society);
    void time_step();
    void run(int time_steps);

  private:
	int current_time_step;
	Society* society;
};
