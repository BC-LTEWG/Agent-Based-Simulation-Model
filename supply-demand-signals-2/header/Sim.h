#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "Society.h"

class Sim {
  public:
    Sim(): currentTimeStep(0) {};
    void timeStep() {};
    void run(int timeSteps) {};

  private:
	int current_time_step;
	Society society;
};
