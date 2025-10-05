#pragma once  
#include <string>
#include <vector>
#include "Worker.h"
#include "Machine.h"
#include <unordered_map>

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
    std::unordered_map<std::string, std::vector<Plan&>> planHistory;

    Firm(int workerCount, std::vector<Machine&> machines, std::vector<Worker&> workers, std::vector<Plan&> plans) : workerCount(workerCount), machines(machines), workers(workers), plans(plans) {}
    
    std::vector<Plan&> addPlan(std::vector<Plan&> newPlans) {};
    std::vector<Machine&> addMachine(std::vector<Machine&> newMachines) {};
    std::vector<Worker&> addWorkers(std::vector<Worker&> newWorkers) {};
    std::vector<double> avgingPlanMetrics(std::string planName) {};

    Machine& getMachine(std::string machineName) {};
    Plan& getPlan(std::string planName) {};

    std::size_t getWorkerCount() {};
    std::size_t getMachineCount() {};
    std::vector<Machine&> getMachines() {};
    std::vector<Worker&> getWorkers() {};
    std::vector<Plan&> getPlans() {};
    float getAvgProductivity() {};



    
};
