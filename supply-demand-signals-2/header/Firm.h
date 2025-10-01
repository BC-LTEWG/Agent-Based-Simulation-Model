#pragma once  
#include <string>
#include <vector>
#include "Worker.h"
#include "Machine.h"

struct Plan {
  std::vector<std::string> fixedCapital;
  int laborHours;
  int rawMaterials;
  int totalQuantity;
  int prd;
};

class Firm {
  public:
    int workerCount;
    std::vector<Machine> machines;
    std::vector<Worker> workers;
    Plan plan;

    Firm(int workerCount, std::vector<Machine> machines, std::vector<Worker> workers, Plan plan) : workerCount(workerCount), machines(machines), workers(workers), plan(plan) {}
};
