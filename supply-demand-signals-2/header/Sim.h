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
    void timeStep() {};
    void run(int timeSteps) {};

  private:
	int current_time_step;
	Society* society;
};
