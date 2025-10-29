#pragma once 
#include "Product.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <random>



enum health_status { HEALTHY, UNHEALTHY, RECOVERING };

enum expertise { EXPERTISE_1, EXPERTISE_2, EXPERTISE_3 };


struct Person {
  std::unordered_map<std::string, int> expertise;
  int age;
  health_status healthStatus;
  std::unordered_map<std::string, double> needs;

  
  Person(const std::unordered_map<std::string, int>& expertise, int age, health_status healthStatus, const std::unordered_map<std::string, double>& needs) : expertise(expertise), age(age), healthStatus(healthStatus), needs(needs) {}
  
  std::unordered_map<std::string, double>& getWorkerNeeds() {return this->needs;};
  health_status getHealthStatus() {return this->healthStatus;};

  float getCurrentProductivity();
  
  float avgProductivityOverTimeStep(std::string productName);

  void purchaseGoods(Product& p, int quantity) {};
};
