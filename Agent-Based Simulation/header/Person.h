#pragma once

#include <unordered_map>
#include <vector>

#include "Agent.h"

struct Product;
class Distributor;
class Firm;
class Society;

class Person : public Agent {
  public:
    enum HealthStatus { HEALTHY, UNHEALTHY };
    enum Ability { ABILITY_1, ABILITY_2, ABILITY_3, NUM_ABILITIES }; 
	
    Person(Society * society);
    unsigned int get_id() override;
	void on_time_step() override;
  
	std::unordered_map<Ability, double>& get_abilities();
	void train(std::unordered_map<Ability, double> target_abilities);
    std::unordered_map<Product*, double>& get_purchase_frequencies();
    HealthStatus get_health_status();
    float get_current_productivity();
    float avg_productivity_over_time_step(std::string product_name);
    void register_hours_worked(double hours_worked);
    bool charge(double cost);
    void purchase_good(Product * p, int quantity);
    void set_firm(Firm *);
  
  private:
    Society * society;
    unsigned int id;
    std::unordered_map<Ability, double> abilities;
    int age;
    HealthStatus health_status;
    std::unordered_map<Product *, double> purchase_frequencies,
        ideal_purchase_quantities;
    Firm * firm = nullptr;
    double account;
	std::vector<Distributor *> ranked_distributors;
	int shopping_offset;

	bool will_shop();
	void shop();
    bool will_retire();
	void retire();
	void update_health_status();

    void log_hours(const double hours);
    void log_shopping(const std::string product_name, int quantity);
    void log_shopping_deficit(double deficit);
    void log_state();
};
