#include "Firm.h"
#include <string>
#include <vector>


std::vector<Plan&> Firm::addPlan(Firm& f, std::vector<Plans&> newPlans) {
  for(const auto& plan: newPlans) {
    f.plans.push_back(plan);
  }
  return f.plans;
}

std::vector<Plan&> Firm::addPlan(Firm& f, std::vector<Plans&> newMachines) {
  for(const auto& machine: newMachines) {
    f.machines.push_back(machines);
  }
  return f.plans;
}

std::vector<Plan&> Firm::addWorkers(Firm& f, std::vector<Workers&> newWorkers) {
  for(const auto& workers: newWorkers) {
    f.workers.push_back(workers);
  }
  return f.plans;
}

std::vector<double> firm::avgingPlanMetrics(Firm& f, std::string planName) {
  if(f.planHistory.find(planName) != f.planHistory.end()) {
    avgRawMaterialCost = std::reduce(f.planHistory[planName].rawMaterials.begin(), f.planHistory[planName].rawMaterials.end(), 0.0) / f.planHistory[planName].size();
    avgLaborCost = std::reduce(f.planHistory[planName].laborHours.begin(), f.planHistory[planName].laborHours.end(), 0.0) / f.planHistory[planName].size();
    std::vector<Machine&> machines = f.planHistory[planName].machines;
    avgMachineCost = std::reduce(machines.begin(), machines.end(), 0.0) / machines.size();
  }
  return {avgRawMaterialCost, avgLaborCost, avgRawMaterialCost};
}
