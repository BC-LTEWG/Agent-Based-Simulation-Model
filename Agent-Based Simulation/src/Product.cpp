#include <random>

#include "Constants.h"
#include "Product.h"
#include "Sim.h"

Product::Product(const std::string name) : product_name{name} {
    static std::uniform_int_distribution<>
        order_size_dist(PRODUCT_ORDER_SIZE_MIN, PRODUCT_ORDER_SIZE_MAX);
    order_size = order_size_dist(Sim::gen);
    static std::uniform_real_distribution<>
        frequency_dist(PRODUCT_FREQUENCY_MIN, PRODUCT_FREQUENCY_MAX);		
    base_frequency = frequency_dist(Sim::gen);
    for (int i = 0; i < NUM_ABILITIES; i++) {
        required_abilities.push_back((Ability) i);
    }
    shuffle(required_abilities.begin(), required_abilities.end(), Sim::gen);
    static std::uniform_int_distribution<> ability_count_dist(1, PRODUCT_ABILITY_COUNT_MAX);
    required_abilities.resize(ability_count_dist(Sim::gen));
}

void Product::add_input(Product * input, int quantity) {
    inputs_per_unit[input] = quantity;
}
