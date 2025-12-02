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
	Order * order;

    Firm * firm;
	std::vector<Person*> workers;
				 
	bool will_train_workers;
	int predicted_turnaround_time;

    int prd;
    int labor_hours;
    int raw_materials;
    int total_hours;
    int labor_hours_remaining;
    int raw_materials_remaining;
    int total_hours_remaining;
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
	int predict_workers_needed(Order * order);
	double predict_turnaround_time(Order * order, double total_suitability); 
	void assign_predicted_turnaround_time(Plan * draft_plan, std::vector<Ability>& required_abilities);
	void assign_workers_by_suitability_threshold(Plan * draft_plan, std::vector<Ability>& required_abilities, double suitability_threshold);
	void assign_workers(Plan * draft_plan, std::vector<Ability>& required_abilities);
};
