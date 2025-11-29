#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Agent.h"
#include "Machine.h"
#include "Person.h"
#include "Product.h"

class Firm;
class Person;

struct Plan {
    Product * product;
    std::vector<Machine*> machines;
	std::vector<Person*> workers;
    Firm * firm;
				 
    int total_quantity;
	int allotted_time;

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
    std::vector<Plan*> plans;
    std::unordered_map<Product*, std::vector<Plan*>> plan_history;
	
    Firm();
    Firm(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans);
    
    double get_avg_productivity();
	int assign_workers(std::vector<Ability>& needed_abilities, int num_workers, Plan& plan);

  private:
	double suitability(std::vector<Ability>& needed_abilities, Person * person);
};
