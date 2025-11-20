#pragma once
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Firm.h"
#include "Person.h"
#include "Producer.h"

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
    void receive_order(Product * product, int quantity);
    void initialize_inventory(Product * product, int quantity);
    int get_inventory(Product * product);

  private:
    std::vector<Producer *> suppliers;
    std::unordered_map<Product *, int> inventory;
    std::unordered_map<Product *, int> reorder_thresholds;
};
