#include <algorithm>
#include <iostream>
#include <random>
#include <set>

#include "Constants.h"
#include "Machine.h"
#include "Product.h"
#include "Sim.h"

struct Machine;

Product::Product(const std::string name) : product_name{name} {
    static std::uniform_int_distribution<>
        order_size_dist(PRODUCT_ORDER_SIZE_MIN, PRODUCT_ORDER_SIZE_MAX);
    order_size = order_size_dist(Sim::get_random_generator());
    static std::uniform_real_distribution<>
        living_labor_dist(
                PRODUCT_LABOR_PER_UNIT_MIN,
                PRODUCT_LABOR_PER_UNIT_MAX
                );
    living_labor_per_unit = (double)living_labor_dist(Sim::get_random_generator());
    for (int i = 0; i < Person::NUM_ABILITIES; i++) {
        required_abilities.push_back((Person::Ability) i);
    }
    std::shuffle(required_abilities.begin(), required_abilities.end(), Sim::get_random_generator());
    static std::uniform_int_distribution<>
        ability_count_dist(1, PRODUCT_ABILITY_COUNT_MAX);
    required_abilities.resize(ability_count_dist(Sim::get_random_generator()));
    static std::uniform_real_distribution<>
        frequency_dist(
                PRODUCT_CONSUMPTION_FREQUENCY_MIN,
                PRODUCT_CONSUMPTION_FREQUENCY_MAX
                );
    mean_consumption_frequency = frequency_dist(Sim::get_random_generator());
}

void Product::set_inputs(std::vector<Product *>& goods) {
    static std::uniform_int_distribution<>
        num_inputs_dist(PRODUCT_NUM_INPUTS_MIN, PRODUCT_NUM_INPUTS_MAX);
    const std::size_t num_inputs = num_inputs_dist(Sim::get_random_generator());
    std::uniform_int_distribution<>
        product_input_index_dist(0, goods.size() - 1);
    std::set<int> indices;
    while (indices.size() < num_inputs) {
        indices.insert(product_input_index_dist(Sim::get_random_generator()));
    }
    static std::uniform_real_distribution<>
        input_per_unit_dist(
                PRODUCT_INPUT_PER_UNIT_MIN,
                PRODUCT_INPUT_PER_UNIT_MAX
                );
    for (int index : indices) {
        inputs_per_unit[goods[index]] = input_per_unit_dist(Sim::get_random_generator());
    }
}

void Product::set_machines(std::vector<Machine*> machines) {
    if (!machines.size()) return;
    static std::uniform_int_distribution<>
        num_machines_dist(PRODUCT_NUM_MACHINES_MIN, PRODUCT_NUM_MACHINES_MAX);
    const std::size_t num_machines = num_machines_dist(Sim::get_random_generator());
    std::uniform_int_distribution<>
        product_machine_index_dist(0, machines.size() - 1);
    std::set<int> indices;
    while (indices.size() < num_machines) {
        indices.insert(product_machine_index_dist(Sim::get_random_generator()));
    }
    for (int index : indices) {
        machines_needed.push_back(machines[index]);
    }
 }
