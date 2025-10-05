#include "Firm.h"
#include <string>
#include <vector>
#include <numeric>


std::vector<Plan&> Firm::addPlan(std::vector<Plan&> newPlans) {
  for(auto& plan: newPlans) {
    this->plans.push_back(plan);
  }
  return this->plans;
}

std::vector<Machine&> Firm::addMachine(std::vector<Machine&> newMachines) {
  for(auto& machine: newMachines) {
    this->machines.push_back(machine);
  }
  return this->machines;
}

std::vector<Worker&> Firm::addWorkers(std::vector<Worker&> newWorkers) {
  for(auto& worker: newWorkers) {
    this->workers.push_back(worker);
  }
  return this->workers;
}

std::vector<double> Firm::avgingPlanMetrics(std::string planName) {
  double avgRawMaterialCost = 0.0;
  double avgLaborCost = 0.0;
  double avgMachineCost = 0.0;
  
  if(this->planHistory.find(planName) != this->planHistory.end()) {
    std::vector<Plan&>& plans = this->planHistory[planName];
    avgRawMaterialCost = std::reduce(plans.begin(), plans.end(), 0.0, [](double sum, const Plan& p){ return sum + p.rawMaterials; }) / plans.size();
    avgLaborCost = std::reduce(plans.begin(), plans.end(), 0.0, [](double sum, const Plan& p){ return sum + p.laborHours; }) / plans.size();
    std::vector<Machine> machines = plans[0].machines;
    avgMachineCost = std::reduce(machines.begin(), machines.end(), 0.0) / machines.size();
  }
  return {avgRawMaterialCost, avgLaborCost, avgMachineCost};
}

std::vector<Machine&> Firm::getMachines() {
  return this->machines;
}

std::vector<Worker&> Firm::getWorkers() {
  return this->workers;
}

std::vector<Plan&> Firm::getPlans() {
  return this->plans;
}

