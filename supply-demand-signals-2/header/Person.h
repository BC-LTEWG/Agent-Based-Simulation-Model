#pragma once

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "Product.h"

class Distributor;
class Firm;
class Society;

class Person {
  public:
    enum HealthStatus { HEALTHY, UNHEALTHY, RECOVERING };

    Person(int age, HealthStatus health_status, std::unordered_map<Ability, double> starting_abilities = {});
  
    std::unordered_map<Product*, double>& get_purchase_frequencies();
    HealthStatus get_health_status();
    float get_current_productivity();
    float avg_productivity_over_time_step(std::string product_name);
    void get_paid(double income);
    void charge(double cost);
    void purchase_good(Product * p, int quantity);
	bool will_shop();
	void shop();
    bool will_retire();
	void retire();
	void set_society(Society * society);
	void on_time_step();
  
  private:
    std::unordered_map<Ability, double> abilities;
    int age;
    HealthStatus health_status;
    std::unordered_map<Product*, double> purchase_frequencies;
    Firm * firm = nullptr;
	Society * society = nullptr;
    double account;
	std::vector<Distributor*> ranked_distributors;
	int shopping_offset;
};
