#include "Constants.h"
#include "Product.h"
#include "Sim.h"
#include <random>

Product::Product(const std::string name, double price, int order) 
    : product_name(name), price_per_unit(price), order_size(order) {
	static std::uniform_real_distribution<> frequency_dist(PRODUCT_FREQUENCY_MIN, PRODUCT_FREQUENCY_MAX);		
	base_frequency = frequency_dist(Sim::gen);

	for (int i = 0; i < NUM_ABILITIES; i++) {
		required_abilities.push_back((Ability) i);
	}
	shuffle(required_abilities.begin(), required_abilities.end(), Sim::gen);
	static std::uniform_int_distribution<> ability_count_dist(1, PRODUCT_ABILITY_COUNT_MAX);
	required_abilities.resize(ability_count_dist(Sim::gen));
}

int Product::get_required_labor() {
    return 1;
}

void Product::add_input(Product & input, int quantity) {
    inputs.push_back(std::unordered_map<Product&, int>({{input, quantity}}));
}