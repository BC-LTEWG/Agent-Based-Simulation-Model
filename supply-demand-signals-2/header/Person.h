#pragma once

#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "Agent.h"
#include "Constants.h"
#include "Product.h"
#include "Society.h"

class Distributor;
class Firm;


class Person : public Agent {
  public:
    enum HealthStatus { HEALTHY, UNHEALTHY, RECOVERING };
	
    Person();
	void on_time_step() override;
  
	std::unordered_map<Ability, double>& get_abilities();
    std::unordered_map<Product*, double>& get_purchase_frequencies();
    HealthStatus get_health_status();
    float get_current_productivity();
    float avg_productivity_over_time_step(std::string product_name);
    void register_hours_worked(double hours_worked);
    void charge(double cost);
    void purchase_good(Product * p, int quantity);
	bool will_shop();
	void shop();
    bool will_retire();
	void retire();
    void set_firm(Firm *);
  
  private:
    std::unordered_map<Ability, double> abilities;
    int age;
    HealthStatus health_status;
    std::unordered_map<Product*, double> purchase_frequencies;
    Firm * firm = nullptr;
    double account;
	std::vector<Distributor*> ranked_distributors;
	int shopping_offset;
};
