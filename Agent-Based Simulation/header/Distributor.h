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

class Distributor : public Firm {
  public:
    Distributor();
    void on_time_step() override;

    double get_output_ratio(Product& product);
    double planned_satisfaction_per_person(Product& product, Person& person);
    void sell_goods(Product& product, int quantity, Person * person);
    bool is_overproduced(Product * product);

  private:
};
