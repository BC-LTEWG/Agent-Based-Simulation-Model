#pragma once

#include <string>
#include <vector>

#include "Constants.h"
#include "Debug.h"
#include "Firm.h"

struct Order;
struct Product;
class Distributor;
struct Machine;
class Person;

class Producer : public Firm {
  public:
    Producer(
        const SET<Product *>& initial_catalog = {}
    );
    Logger::Client get_client_type() override;
    void add_to_catalog(Product * product) override;
    void initialize_inventory() override;
    bool can_produce(Product * product);
	Order * draft_plan_and_return_order(const Order * order);
	void drop_order(Firm * customer);
	void pursue_order(Firm * customer);

  private:
    MAP<Firm *, Plan *> customer_to_draft_plan;

    double get_max_order_quantity(const Order * order);
    void log_draft_plan(const Plan * draft_plan);
    void log_dropped_order(const Order * order);
    
};
