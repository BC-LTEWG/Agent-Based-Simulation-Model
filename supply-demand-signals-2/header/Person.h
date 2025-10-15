#pragma once 
#include <string>
#include <vector>
#include <unordered_map>
#include <random>

enum HEALTH_STATUS { HEALTHY, UNHEALTHY, RECOVERING };


struct Person {
  std::unordered_map<std::string, int> expertise;
  int age;
  HEALTH_STATUS healthStatus;
  std::vector<std::string> needs;

  
  Person(const std::unordered_map<std::string, int>& expertise, int age, HEALTH_STATUS healthStatus, std::vector<std::string> needs) : expertise(expertise), age(age), healthStatus(healthStatus), needs(needs) {}
  
  std::vector<std::string>& getWorkerNeeds() {return this->needs;};
  HEALTH_STATUS getHealthStatus() {return this->healthStatus;};

  float getCurrentProductivity();
  
  float avgProductivityOverTimeStep(std::string productName);
};
