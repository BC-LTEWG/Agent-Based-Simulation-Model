#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "Firm.h"
#include "Machine.h"
#include "Person.h"
#include "Product.h"

class Distributor;

struct Order {
    Product * product;
    int quantity;
    Distributor * customer;
    int requested_turnaround_time;
};

class Producer : public Firm {
  public:
    Producer();
    Producer(std::vector<Machine *> machines, std::vector<Person *> workforce);
    Producer(std::vector<Machine *> machines, std::vector<Person *> workforce, std::vector<Plan *> plans);
    
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
