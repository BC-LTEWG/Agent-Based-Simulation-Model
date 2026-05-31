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
    Distributor(
        Society * society,
        const std::unordered_set<Product *>& initial_catalog
    );
    Logger::Client get_client_type() override;
    void on_time_step() override;
    int try_sell_goods(ConsumerGood * consumer_good, int quantity, Person * person);
    
  private:
    double get_pending_inventory_level(Product * product) override;
    void log_shortfall(unsigned int product_id, int shortfall);
};
