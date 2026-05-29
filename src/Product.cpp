#include <algorithm>
#include <iostream>
#include <random>
#include <set>

#include "Constants.h"
#include "Good.h"
#include "Logger.h"
#include "Machine.h"
#include "Product.h"
#include "Sim.h"

struct Machine;

Product::Product() {
    static unsigned int unique_id = 0;
    id = unique_id++;
    static std::uniform_real_distribution<>
        living_labor_dist(
                PRODUCT_LABOR_PER_UNIT_MIN,
                PRODUCT_LABOR_PER_UNIT_MAX
                );
    living_labor_per_unit = living_labor_dist(Sim::get_random_generator());
    Society::get_instance()->
        set_underlying_living_labor_per_unit(this, living_labor_per_unit);
    for (Ability * ability : Society::get_instance()->get_abilities()) {
        required_abilities.push_back(ability);
    }
    std::shuffle(required_abilities.begin(), required_abilities.end(), Sim::get_random_generator());
    static std::uniform_int_distribution<>
        ability_count_dist(1, PRODUCT_ABILITY_COUNT_MAX);
    required_abilities.resize(ability_count_dist(Sim::get_random_generator()));
}

void Product::set_inputs() {
    std::vector<Good *> goods = Society::get_instance()->get_goods();
    int max_num_inputs = std::min<int>(PRODUCT_NUM_INPUTS_MAX, goods.size());
    static std::uniform_int_distribution<>
        num_inputs_dist(PRODUCT_NUM_INPUTS_MIN, max_num_inputs);
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

void Product::set_machines() {
    std::vector<Machine *> machines = Society::get_instance()->get_machines();
    if (!machines.size()) {
        return;
    }
    const unsigned int global_num_machines =
        Sim::get_num_products() / Sim::get_products_per_machine();
    const int num_machines_max =
        global_num_machines / MAX_PROPORTION_OF_MACHINES_PER_PRODUCT;
    static std::uniform_int_distribution<>
        num_machines_dist(PRODUCT_NUM_MACHINES_MIN, num_machines_max);
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
