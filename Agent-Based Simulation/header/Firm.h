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

	// dependent/output fields	
	int predicted_turnaround_time;
    double machinery_cost;
    int labor_hours;
    double raw_materials;
    double total_hours;
    double prd;
    int labor_hours_remaining;
    double raw_materials_remaining;
    double total_hours_remaining;
    double quantity_remaining;
	int outgoing_units_consumed;
};

struct Order {
    enum OrderStatus { ORDER_REQUESTED, ORDER_IN_PROGRESS, ORDER_FINISHED };
    Product * product;
    int quantity;
    Firm * customer;
    int requested_turnaround_time;
    OrderStatus status;
    
    Order(
            Product * product,
            int quantity,
            Firm * customer,
            int requested_turnaround_time
    );
};

struct DemandSignal {
    int quantity;
    unsigned int timestep;
};

class Firm : public Agent {
  public:
    Firm(Society * society, std::unordered_set<Product *> initial_catalog);
    unsigned int get_id() override;
    virtual void on_time_step() override;

    void initialize_inventory(std::unordered_map<Product *, int>& inventory_items);
    
    int get_inventory(Product * product);
    void add_supplier(Producer * producer);
    void receive_shipment(Order * order);

  protected:
    Society * society;
    unsigned int id;
    std::vector<Machine *> machines;
    std::vector<Person *> workers;
	
    std::vector<Producer *> suppliers;
    std::unordered_map<Product *, int> inventory;
    std::unordered_set<Product *> catalog;
    
    std::unordered_map<Product *, std::queue<DemandSignal>> demand_signals;
    std::unordered_map<Product *, int> total_demands;
    std::unordered_map<Product *, std::unordered_set<Order *>> product_to_outbound_orders;
    std::unordered_map<Product *, double> local_living_labor_per_unit;
    std::vector<Plan *> plans_in_progress;

    Producer * send_order(Order * order);
    double get_reorder_threshold(Product * product);
    int get_pending_output_inventory(Product * product);
    void reorder_output_product_to_threshold(
        Product * product, 
        double threshold,
        int pending_inventory
    );
    void check_and_reorder();

	int predict_workers_needed(Order * order);
    void assign_workers(
        Plan * draft_plan,
        std::vector<Person::Ability>& required_abilities
    );
	int predict_turnaround_time(Order * order, std::vector<Person*>& workers); 
	int predict_labor_hours(Order * order, std::vector<Person*>& workers);
	void assign_plan_dependent_fields(Plan * draft_plan, std::vector<Person::Ability>& required_abilities);
	void draft_optimal_plan(Plan * draft_plan, std::vector<Person::Ability>& required_abilities);
	void train_workers(std::vector<Person *>& workers, std::vector<Person::Ability>& required_abilities);
    void add_demand_signal(Product * product, int quantity);
    void apply_demand_window();
    double get_demand(Product * product);
    virtual std::unordered_set<Product *> get_products_to_reorder() = 0;

    void log_shipment_received(std::string product_name, int quantity);
    void log_reorder(std::string product_name, int quantity);
    void log_accepted_order(std::string product_name, int requested_turnaround_time);
    void log_demand(std::string product_name, double demand);
};
