#pragma once 
#include "Product.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <random>





struct Person {
    enum HealthStatus { HEALTHY, UNHEALTHY, RECOVERING };
    enum Expertise { EXPERTISE_1, EXPERTISE_2, EXPERTISE_3 };

    Person(
            const std::unordered_map<std::string, int>& Expertise,
            int age,
            HealthStatus health_status,
            const std::unordered_map<std::string, double>& needs);

    std::unordered_map<std::string, double>& getWorkerNeeds() {return this->needs;};
    HealthStatus getHealthStatus() {return this->health_status;};

    float getCurrentProductivity();

    float avgProductivityOverTimeStep(std::string productName);

    void purchaseGoods(Product& p, int quantity) {};
    std::unordered_map<std::string, int> expertise;
    int age;
    HealthStatus health_status;
    std::unordered_map<std::string, double> needs;

};
