#pragma once 
#include <vector>
#include <string>
#include "Society.h"

public Sim {
  public:
    Sim(): timeStep(0) {};
    void advanceTimeStep() {};
    void runSim(int days) {};
    void runWorkDay() {};
    void resetTimeStep() {timeStep = std::chrono::hours(0);};


  private:
    std::chrono::hours timeStep;



}
