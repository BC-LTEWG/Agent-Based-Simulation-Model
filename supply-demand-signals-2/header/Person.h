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
  std::unordered_map<std::string, int> needs;

  
  Person(const std::unordered_map<std::string, int>& expertise, int age, HEALTH_STATUS healthStatus, const std::unordered_map<std::string, int>& needs) : expertise(expertise), age(age), healthStatus(healthStatus), needs(needs) {}
  
  std::unordered_map<std::string, int>& getWorkerNeeds() {return this->needs;};
  HEALTH_STATUS getHealthStatus() {return this->healthStatus;};

  float getCurrentProductivity();
  
  float avgProductivityOverTimeStep(std::string productName);

  void purchaseGoods(Product& p, int quantity) {};
};
