#include <random>
#include <set>

#include "Constants.h"
#include "Product.h"
#include "Sim.h"

Product::Product(const std::string name) : product_name{name} {
    static std::uniform_int_distribution<>
        order_size_dist(PRODUCT_ORDER_SIZE_MIN, PRODUCT_ORDER_SIZE_MAX);
    order_size = order_size_dist(Sim::gen);
    static std::uniform_int_distribution<>
        living_labor_dist(
                PRODUCT_LABOR_PER_UNIT_MIN,
                PRODUCT_LABOR_PER_UNIT_MAX
                );
    living_labor_per_unit = (double)living_labor_dist(Sim::gen);
    for (int i = 0; i < Person::NUM_ABILITIES; i++) {
        required_abilities.push_back((Person::Ability) i);
    }
    shuffle(required_abilities.begin(), required_abilities.end(), Sim::gen);
    static std::uniform_int_distribution<>
        ability_count_dist(1, PRODUCT_ABILITY_COUNT_MAX);
    required_abilities.resize(ability_count_dist(Sim::gen));
    static std::uniform_real_distribution<>
        frequency_dist(
                PRODUCT_CONSUMPTION_FREQUENCY_MIN,
                PRODUCT_CONSUMPTION_FREQUENCY_MAX
                );
    mean_consumption_frequency = frequency_dist(Sim::gen);
}

void Product::set_inputs(std::vector<Product *>& products, int product_index) {
    static std::uniform_int_distribution<>
        num_inputs_dist(PRODUCT_NUM_INPUTS_MIN, PRODUCT_NUM_INPUTS_MAX);
    const int num_inputs = num_inputs_dist(Sim::gen);
    std::uniform_int_distribution<>
        product_index_dist(0, products.size() - 1);
    std::set<int> indices;
    while (indices.size() < num_inputs) {
        int index = product_index_dist(Sim::gen);
        if (index != product_index) {
            indices.insert(product_index_dist(Sim::gen));
        }
    }
    static std::uniform_int_distribution<>
        num_input_units_dist(
                PRODUCT_NUM_INPUT_UNITS_MIN,
                PRODUCT_NUM_INPUT_UNITS_MAX
                );
    for (int index : indices) {
        inputs_per_unit[products[index]] = num_input_units_dist(Sim::gen);
    }
}
