#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "Constants.h"
#include "Firm.h"

struct Order;
struct Product;
class Distributor;
class Machine;
class Person;

class Producer : public Firm {
  public:
    Producer();
    Producer(std::vector<Machine *> machines, std::vector<Person *> workforce);
    Producer(std::vector<Machine *> machines, std::vector<Person *> workforce, std::vector<Plan *> plans);
    void on_time_step() override;
    
    void add_product_to_catalog(Product * product);
    bool can_produce(Product * product);
    std::set<Product *>& get_catalog();
    Order * accept_order(Product * product, int quantity, Distributor * customer);
    void process_orders();
    void complete_order(Order * order);

  private:
    std::set<Product *> products;
    std::unordered_map<Product *, int> inventory;
    std::vector<Order *> pending_orders;
};
