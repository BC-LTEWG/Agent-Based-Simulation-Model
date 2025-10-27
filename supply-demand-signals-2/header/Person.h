#pragma once

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "Firm.h"
#include "Product.h"
#include "Society.h"


class Person {
  public:
    enum HealthStatus { HEALTHY, UNHEALTHY, RECOVERING };
    enum Expertise { EXPERTISE_1, EXPERTISE_2, EXPERTISE_3 };

    Person(
        const std::unordered_map<std::string, int>& expertise,
        int age,
        HealthStatus health_status,
        const std::unordered_map<std::string, double>& needs);

    std::unordered_map<std::string, double>& get_worker_needs();
    HealthStatus get_health_status();
    float get_current_productivity();
    float avg_productivity_over_time_step(std::string product_name);
    void purchase_goods(Product& p, int quantity);
    void get_paid(double income);
    void charge(double cost);
    bool willRetire();

  private:
    std::unordered_map<std::string, int> expertise;
    int age;
    HealthStatus health_status;
    std::unordered_map<std::string, double> needs;
	Firm employer;
    double account;
};
