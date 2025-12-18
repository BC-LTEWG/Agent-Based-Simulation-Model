#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Agent.h"
#include "Constants.h"
#include "Person.h"

class Firm;
struct Machine;
struct Order;
class Producer;
struct Product;

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

struct Order {
    Product * product;
    int quantity;
    Firm * customer;
    int requested_turnaround_time;
};

class Firm : public Agent {
  public:
    std::vector<Machine*> machines;
    std::vector<Person*> workers;
	
	Firm();

    void initialize_inventory(std::unordered_map<Product *, int>& inventory_items);
    
    double get_avg_productivity();
	bool has_product(Product * product);
    int get_inventory(Product * product);
    void add_supplier(Producer * producer);
    void set_reorder_threshold(Product * product, int threshold);
    void receive_shipment(Product * product, int quantity);

  protected:
    std::vector<Producer *> suppliers;
    std::unordered_map<Product *, int> inventory;
    std::unordered_map<Product *, int> reorder_thresholds;
    std::unordered_map<Product*, std::vector<Plan*>> plan_history; // unused and prob need to change later
    std::vector<Plan*> plans_in_progress;

    Producer * find_producer_for_product(Product * product);
    Producer * send_order(Order * order);
    void check_and_reorder();

	double suitability(Person * person, std::vector<Ability>& required_abilities);
	double suitability(std::unordered_map<Ability, double>& abilities, 
			           std::vector<Ability>& required_abilities,
					   float productivity);
	int predict_workers_needed(Order * order);
	void assign_workers_by_suitability_threshold(Plan * draft_plan, std::vector<Ability>& required_abilities, double suitability_threshold);
	int predict_turnaround_time(Order * order, double total_suitability); 
	int predict_labor_hours(Order * order, double total_suitability);
	void assign_plan_dependent_fields(Plan * draft_plan, std::vector<Ability>& required_abilities);
	void draft_optimal_plan(Plan * draft_plan, std::vector<Ability>& required_abilities);
	void train_workers(std::vector<Person *>& workers, std::vector<Ability>& required_abilities);
};
