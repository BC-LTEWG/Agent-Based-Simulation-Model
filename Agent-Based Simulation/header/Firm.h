#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

#include "Agent.h"
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
    int labor_hours;
    double raw_materials;
    double total_hours;
    double prd;
    double m;
    int labor_hours_remaining;
    double raw_materials_remaining;
    double total_hours_remaining;
	int outgoing_units_consumed;
};

struct Order {
    enum OrderStatus { ORDER_REQUESTED, ORDER_FINISHED };

    Product * product;
    int quantity;
    Firm * customer;
    int requested_turnaround_time;
    OrderStatus status;
    
    Order(Product * p,
          int q,
          Firm * c,
          int turnaround)
        : product(p),
          quantity(q),
          customer(c),
          requested_turnaround_time(turnaround),
          status(ORDER_REQUESTED)
    {}


};

struct DemandSignal {
    Product * product;
    int quantity;
    int timestep;
};

class Firm : public Agent {
  public:
	Firm(Society * society);
    Firm(Society * society, std::unordered_set<Product *> initial_catalog);
    virtual void on_time_step() override;

    void initialize_inventory(std::unordered_map<Product *, int>& inventory_items);
    
    double get_avg_productivity();
	bool has_product(Product * product);
    int get_inventory(Product * product);
    void add_supplier(Producer * producer);
    void receive_shipment(Order * order);

  protected:
    Society * society;
    std::vector<Machine*> machines;
    std::vector<Person*> workers;
	
    std::vector<Producer *> suppliers;
    std::unordered_map<Product *, int> inventory;
    std::unordered_set<Product *> catalog;
    
    std::queue<DemandSignal> demand_signals;
    std::unordered_map<Product *, double> inventory_demands;
    std::unordered_map<Product *, std::unordered_set<Order *>> product_to_outbound_orders;
    std::unordered_map<Product *, std::vector<Plan *>> plan_history; // unused and prob need to change later
    std::vector<Plan *> plans_in_progress;

    Producer * send_order(Order * order);
    double get_reorder_threshold(Product * product);
    int get_pending_inventory(Product * product);
    void reorder_product_to_threshold(
        Product * product, 
        double threshold,
        int pending_inventory
    );
    void check_and_reorder();

	double suitability(Person * person, std::vector<Person::Ability>& required_abilities);
	double suitability(std::unordered_map<Person::Ability, double>& abilities, 
			           std::vector<Person::Ability>& required_abilities,
					   float productivity);
	int predict_workers_needed(Order * order);
    void assign_workers_by_suitability_threshold(
            Plan * draft_plan,
            std::vector<Person::Ability>& required_abilities,
            double suitability_threshold
            );
	int predict_turnaround_time(Order * order, double total_suitability); 
	int predict_labor_hours(Order * order, double total_suitability);
	void assign_plan_dependent_fields(Plan * draft_plan, std::vector<Person::Ability>& required_abilities);
	void draft_optimal_plan(Plan * draft_plan, std::vector<Person::Ability>& required_abilities);
	void train_workers(std::vector<Person *>& workers, std::vector<Person::Ability>& required_abilities);
    void add_demand_signal(Product * product, int quantity);
    void apply_demand_window();
    virtual std::unordered_set<Product *> get_products_to_reorder() = 0;
};
