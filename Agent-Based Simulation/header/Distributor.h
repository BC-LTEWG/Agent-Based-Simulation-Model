#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Firm.h"

struct Product;
class Distributor;
class Person;
class Producer;

class Distributor : public Firm {
  public:
    Distributor();
    Distributor(std::unordered_set<Product *> initial_catalog);
    void on_time_step() override;

    void sell_goods(Product& product, int quantity, Person * person);

  private:
    std::unordered_set<Product *> get_products_to_reorder() override;
};
