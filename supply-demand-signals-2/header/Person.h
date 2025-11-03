#pragma once

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "Firm.h"
#include "Product.h"
#include "Society.h"

class Firm;

class Person {
  public:
    enum HealthStatus { HEALTHY, UNHEALTHY, RECOVERING };
    enum Expertise { EXPERTISE_1, EXPERTISE_2, EXPERTISE_3 };

    Person(const std::unordered_map<std::string, int>& expertise, int age, HealthStatus health_status, const std::unordered_map<Product*, double>& purchase_frequencies);
  
    std::unordered_map<Product*, double>& getWorkerPurchasePeriods() { return this->purchase_frequencies; };
    HealthStatus get_health_status();
    float get_current_productivity();
    float avg_productivity_over_time_step(std::string product_name);
    void get_paid(double income);
    void charge(double cost);
    void purchase_good(Product* p, int quantity);
    void purchase_goods();
    bool will_retire();
  
  private:
    std::unordered_map<std::string, int> expertise;
    int age;
    HealthStatus health_status;
    std::unordered_map<Product*, double> purchase_frequencies;
    Firm* employer;
    double account;
};
