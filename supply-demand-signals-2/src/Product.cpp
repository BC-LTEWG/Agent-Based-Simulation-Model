#include "Constants.h"
#include "Product.h"
#include "Sim.h"

Product::Product(const std::string name, double price, int order) 
    : product_name(name), price_per_unit(price), order_size(order) {
	static std::uniform_real_distribution<> frequency_dist(PRODUCT_FREQUENCY_MIN, PRODUCT_FREQUENCY_MAX);		
	base_frequency = frequency_dist(Sim::gen);

	required_abilities = Sim::get_all_abilities();
	shuffle(required_abilities.begin(), required_abilities.end(), Sim::gen);
	static std::uniform_int_distribution<> ability_count_dist(1, required_abilities.size());
	required_abilities.resize(ability_count_dist(Sim::gen));
}
