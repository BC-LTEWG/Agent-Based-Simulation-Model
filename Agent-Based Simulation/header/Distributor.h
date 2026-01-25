#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Firm.h"

#define PRODUCTION_THRESHOLD 1.5

struct Plan;
struct Product;
class Distributor;
class Person;
class Producer;

class Distributor : public Firm {
  public:
    Distributor(Society * society);
    Distributor(Society * society, std::unordered_set<Product *> initial_catalog);
    void on_time_step() override;

    double get_output_ratio(Product& product);
    double planned_satisfaction_per_person(Product& product, Person& person);
    void sell_goods(Product& product, int quantity, Person * person);
    bool is_overproduced(Product * product);

  private:
    std::unordered_set<Product *> get_products_to_reorder() override;
    std::unordered_map<Product *, Plan *> product_to_plan;
};
