#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Firm.h"

constexpr double PRODUCTION_THRESHOLD = 1.5;
struct Product;
class Distributor;
class Person;
class Producer;

class Distributor : public Firm {
  public:
    Distributor(Society * society);
    Distributor(
        Society * society,
        const std::unordered_set<Product *>& initial_catalog,
        const std::unordered_map<Product *, int>& initial_output_inventory
    );
    void on_time_step() override;
    bool try_sell_goods(Product& product, int quantity, Person * person);
    int get_inventory(Product * product) override;
    void receive_shipment(Order * order) override;

  private:
    int get_pending_input_inventory(Product * product) override;
    int get_pending_output_inventory(Product * product);
    std::unordered_set<Product *> get_products_to_reorder() override;
    std::unordered_map<Product *, int> output_inventory;
    std::unordered_map<Product *, Plan *> product_to_plan;
    void check_expand_catalog();
    void log_shortfall(std::string product_name, int shortfall);
    void log_catalog_size(int size);
};
