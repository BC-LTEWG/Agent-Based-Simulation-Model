#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Firm.h"

#define PRODUCTION_THRESHOLD 1.5

struct Product;
class Distributor;
class Person;
class Producer;

struct Order {
    Product * product;
    int quantity;
    Distributor * customer;
    int requested_turnaround_time;
};

class Distributor : public Firm {
  public:
    Distributor();
    Distributor(std::vector<Machine *> machines, std::vector<Person *> workforce, std::vector<Plan *> plans);
    double get_output_ratio(Product& product);
    double planned_satisfaction_per_person(Product& product, Person& person);
	  bool has_product(Product * product);
    void sell_goods(Product& product, int quantity, Person * person);
    void add_supplier(Producer * producer);
    void set_reorder_threshold(Product * product, int threshold);
    void check_and_reorder();
    Producer * find_producer_for_product(Product * product);
    void receive_shipment(Product * product, int quantity);
    void initialize_inventory(std::unordered_map<Product *, int>& inventory_items);
    int get_inventory(Product * product);
    bool is_overproduced(Product * product);
    void on_time_step() override;

  private:
    std::vector<Producer *> suppliers;
    std::unordered_map<Product *, int> inventory;
    std::unordered_map<Product *, int> reorder_thresholds;
};
