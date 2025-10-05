#pragma once 
#include <string>
#include <vector>

enum HEALTH_STATUS { HEALTHY, UNHEALTHY, RECOVERING };


struct Worker {
  std::string expertise;
  int age;
  HEALTH_STATUS healthStatus;
  std::vector<std::string> interests;
  std::vector<std::string> needs;
  
  Worker(const std::string& expertise, int age, HEALTH_STATUS healthStatus, std::vector<std::string> interests, std::vector<std::string> needs) : expertise(expertise), age(age), healthStatus(healthStatus), interests(interests), needs(needs) {}
  
  Worker& getWorkerNeeds() {return this->needs;};
  Worker& getHealthStatus() {};
  
  Worker& avgProoducitivtyOverTimeStep() {};
};
