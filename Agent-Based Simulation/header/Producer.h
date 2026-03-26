#pragma once

#include <unordered_set>
#include <string>
#include <unordered_map>
#include <vector>

#include "Constants.h"
#include "Firm.h"

struct Order;
struct Product;
class Distributor;
struct Machine;
class Person;

class Producer : public Firm {
  public:
    Producer(
        Society * society,
        const std::unordered_set<Product *>& initial_catalog,
        const std::unordered_map<Product *, int>& input_inventory
    );
    void on_time_step() override;

    bool can_produce(Product * product);
	int draft_plan(Order * order);
	void drop_order(Order * order);
	bool pursue_order(Order * order);
    double get_input_products_account();

  private:
    std::unordered_map<Order *, Plan *> order_to_draft_plan;
    bool has_sufficient_inputs_for_order(const Order * order);
    Plan * create_draft_plan_for_order(Order * order);
    double calculate_machinery_cost_for_plan(const Plan * draft_plan) const;
    void add_order_input_demand_signals(const Order * order);
    void remove_workers_from_available_pools(
        const std::vector<Person *>& assigned_workers
    );
    double calculate_quantity_produced_from_worker_suitability(Plan * plan);
    void register_worked_hour_for_plan_workers(Plan * plan);
    void apply_plan_progress_after_work_step(
        Plan * plan,
        int labor_hours_done,
        double raw_materials_used,
        double quantity_produced
    );
    bool is_within_work_schedule() const;

	void start_plan(Plan * plan);
	void move_plan_forward_one_step(Plan * plan);
	void end_plan(Plan * plan);
	void move_plans_forward_one_step();
    std::unordered_set<Product *> get_products_to_reorder() override;
    void log_plans();
    void log_draft_plan(const Plan * draft_plan);
    void log_dropped_order(const Order * order);
    void log_pursued_plan(const Plan * draft_plan);
    void log_ended_plan(const Plan * plan);
    void log_pending_input_inventory_level(Product * product);
};
