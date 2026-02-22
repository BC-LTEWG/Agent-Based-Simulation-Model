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
    Producer(Society * society, std::unordered_set<Product *> initial_catalog);
    void on_time_step() override;

    bool can_produce(Product * product);
	int draft_plan(Order * order);
	void drop_order(Order * order);
	bool pursue_order(Order * order);
  void reorder_raw_materials(Plan * plan, Product * product);
  bool has_raw_materials();
  void initialize_input_inventory(std::unordered_map<Product *, int>& inventory_items); 
  void get_pending_input_inventory(Product * product);
  int get_input_products_account();

  private:
  std::unordered_map<Product *, int> input_inventory;
	std::unordered_map<Order *, Plan *> order_to_draft_plan;
  int input_products_account;

	void start_plan(Plan * plan);
	void move_plan_forward_one_step(Plan * plan);
	void end_plan(Plan * plan);
	void move_plans_forward_one_step();
    std::unordered_set<Product *> get_products_to_reorder() override;
    void log_plans();
    void log_draft_plan(const Plan * draft_plan);
    void log_dropped_order(const Order * order);
    void log_pursued_plan(const Plan * draft_plan); 
    void log_pending_input_inventory_level(Product * product);
};
