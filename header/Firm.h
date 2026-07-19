#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

#include "Agent.h"
#include "Logger.h"
#include "Person.h"

struct Ability;
struct Machine;
struct Order;
struct Product;
class Firm;
class Producer;
class Society;

class Firm : public Agent {
  public:
    Firm();
    unsigned int get_id() override;
    virtual Logger::Client get_client_type() = 0;
    virtual void on_time_step() override;
    virtual void add_to_catalog(Product * product) = 0;
    double get_avg_productivity();
    double get_inventory_level(Product * product);
    void receive_shipment(Order * order);
    void receive_shipment(Plan * plan);
    void receive_payment(Plan * plan, double transaction_amount);
    double get_busyness();
    double get_pooled_input_value();
    std::vector<Person *> propose_transfer(int workers_wanted);
    void finalize_transfer(Person * worker);


  protected:
    unsigned int id;
    double pooled_input_value = 0.0;
    std::unordered_set<Machine *> machines;
    std::unordered_set<Person *> workers,
        standby_workers;
	
    std::unordered_map<Product *, double> input_inventory;
    std::unordered_set<Product *> catalog;
    
    std::unordered_map<Product *, double> demands;
    std::unordered_map<Product *, std::unordered_set<Order *>> product_to_outbound_orders;
    std::unordered_map<Product *, double> recorded_living_labor_per_unit;
    std::unordered_set<Plan *> plans_in_progress;

    Producer * send_order(Order * order);
    bool remove_input_from_inventory(Product * product, double quantity);
    bool remove_input_from_inventory(
        Product * product,
        double quantity,
        std::vector<std::pair<Product *, double>>& deducted_inputs
    );
    double get_reorder_threshold(Product * product);
    double get_needed_resupply_rate(Product * product);
    virtual void check_and_reorder_input(Product * product);
	void start_plan(Plan * plan);
	void move_plan_forward_one_step(Plan * plan);
	void end_plan(Plan * plan);
	void move_plans_forward_one_step();
    double calculate_quantity_produced_from_worker_suitability(Plan * plan);
    bool is_within_work_schedule() const;
    void rollback_plan_inputs(
        Plan * plan,
        const std::vector<std::pair<Product *, double>>& deducted_inputs
    );

	int predict_workers_needed(Plan * plan);
    void assign_workers(Plan * draft_plan);
	double predict_turnaround_time(Plan * plan); 
	double predict_labor_hours(Order * order, std::vector<Person*>& workers);
    double calculate_raw_material_cost_for_order(Order * order);
    void initialize_plan_budget(Plan * draft_plan);
    double calculate_machinery_cost_for_plan(Plan * draft_plan);
	void assign_plan_dependent_fields(Plan * draft_plan);
    void add_demand_signal(Product * product, double quantity);
    Plan * draft_plan_for_order(Order * order); 
    void update_demands();
    void move_worker_off_standby(Person * worker);

    void log_plans();
    void log_pursued_plan(const Plan * draft_plan);
    void log_ended_plan(const Plan * plan);
    void log_shipment_received(const Product * product, const double quantity);
    void log_inventory_level(const Product * product, const double quantity);
    void log_inventory_reduction(const Product * product, const double quantity);
    void log_reorder(const Product * product, int quantity);
    void log_initial_employment(const unsigned int worker_id, const unsigned int workplace_id);
    void log_busyness(double firm_busyness, double societal_busyness, int max_workers_for_transfer);
    void log_employment_transfer(
            const unsigned int worker_id,
            const unsigned int old_workplace_id,
            const unsigned int new_workplace_id
            );
    void log_reorder_failure(const Product * product, int quantity);
    void log_transfer_request();
    void log_product_quantity(
            const char * const label,
            const Product * product,
            const double quantity
            );
    void log_accepted_order(const Order * original_order, const Order * chosen_return_Order);
    void log_demand(const Product * Product, double demand);
    void log_catalog();
    void log_catalog_addition(Product * product);
};
