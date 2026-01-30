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
    Producer(Society * society);
    Producer(Society * society, std::unordered_set<Product *> initial_catalog);
    void on_time_step() override;

    bool can_produce(Product * product);
	int draft_plan(Order * order);
	bool drop_order(Order * order);
	bool pursue_order(Order * order);

  private:
	std::unordered_map<Order *, Plan *> order_to_draft_plan;
	void start_plan(Plan * plan);
	void execute_plan(Plan * plan);
	void end_plan(Plan * plan);
	void execute_plans();
    std::unordered_set<Product *> get_products_to_reorder() override;
};
