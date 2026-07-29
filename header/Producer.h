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
    inline static bool wiretripped = false;
    Producer(
        const std::unordered_set<Product *>& initial_catalog = {}
    );
    Logger::Client get_client_type() override;
    void add_to_catalog(Product * product) override;
    void initialize_inventory() override;
    bool can_produce(Product * product);
	Order * draft_plan_and_return_order(const Order * order);
	void drop_order(Firm * customer);
	void pursue_order(Firm * customer);

  private:
    std::unordered_map<Firm *, Plan *> customer_to_draft_plan;

    double get_max_order_quantity(const Order * order);
    void adjust_workers_for_quantity(Plan * draft_plan);
    void log_draft_plan(const Plan * draft_plan);
    void log_dropped_order(const Order * order);
    
};
