#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "Firm.h"
#include "Debug.h"

struct Product;
class Distributor;
class Person;
class Producer;

class Distributor : public Firm {
  public:
    Distributor(
        const SET<Product *>& initial_catalog = {}
    );
    Logger::Client get_client_type() override;
    void add_to_catalog(Product * product) override;
    void initialize_inventory() override;
    int try_sell_goods(ConsumerGood * consumer_good, int quantity, Person * person);
    
  private:
    void check_and_reorder_input(Product * product) override;
    void log_shortfall(unsigned int product_id, int shortfall);
};
