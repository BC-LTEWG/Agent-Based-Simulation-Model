#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Agent.h"
#include "Constants.h"

struct Order;
struct Product;
class Firm;
class Machine;
class Person;

struct Plan {
	// independent/input fields
	Order * order;
    Firm * firm;
	std::vector<Person*> workers;
	int training_time;
	int training_time_remaining;

	// dependent/output fields	
	int predicted_turnaround_time;
    double prd;
    int labor_hours;
    double raw_materials;
    double total_hours;
    double m;
    int labor_hours_remaining;
    double raw_materials_remaining;
    double total_hours_remaining;
};

class Firm : public Agent {
  public:
    std::vector<Machine*> machines;
    std::vector<Person*> workers;
	
	Firm();
    
    double get_avg_productivity();

  protected:
    std::unordered_map<Product *, int> inventory;
    std::unordered_map<Product*, std::vector<Plan*>> plan_history; // unused and prob need to change later
    std::vector<Plan*> plans_in_progress;

	double suitability(Person * person, std::vector<Ability>& required_abilities);
	double suitability(std::unordered_map<Ability, double>& abilities, std::vector<Ability>& required_abilities);
	int predict_workers_needed(Order * order);
	void assign_workers_by_suitability_threshold(Plan * draft_plan, std::vector<Ability>& required_abilities, double suitability_threshold);
	int predict_turnaround_time(Order * order, double total_suitability); 
	int predict_labor_hours(Order * order, double total_suitability);
	void assign_plan_dependent_fields(Plan * draft_plan, std::vector<Ability>& required_abilities);
	void draft_optimal_plan(Plan * draft_plan, std::vector<Ability>& required_abilities);
	void train_workers(std::vector<Person *>& workers, std::vector<Ability>& required_abilities);
};
