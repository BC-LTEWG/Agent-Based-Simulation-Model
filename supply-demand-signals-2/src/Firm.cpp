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

std::vector<Plan&> firm::avgingPlanMetrics(Firm& f, Plan& p) {
  for(const auto& plan : f.plans) {
    if(plan == p) {

    }
  }
}
