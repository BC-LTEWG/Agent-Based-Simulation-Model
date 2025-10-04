#pragma once  
#include <string>
#include <vector>
#include "Worker.h"
#include "Machine.h"

struct Plan {
  std::string planName;
  std::vector<Machine> machines;
  int laborHours;
  int rawMaterials;
  int totalQuantity;
  int prd;
};

class Firm {
  public:
    int workerCount;
    std::vector<Machine&> machines;
    std::vector<Worker&> workers;
    std::vector<Plan&> plans;

    Firm(int workerCount, std::vector<Machine&> machines, std::vector<Worker&> workers, std::vector<Plan&> plans) : workerCount(workerCount), machines(machines), workers(workers), plan(plan) {}
    
    std::vector<Plan&> addPlan(std::vector<Plans&> newPlans) {};
    std::vector<Machine&> addMachine(std::vector<Machine&> machines) {};
    std::vector<Workers&> addWorkers(std::vector<Workers&> workers) {};
    std::vector<Plans&> avgingPlanMetrics() {};

    Machine& getMachine(std::strting machineName) {};
    Plan& getPlan(std::string planName) {};

    std::size_t getWorkerCount() {};
    std::size_t getMachineCount() {};
    std::size_t getPlanCount() {};



    
};
