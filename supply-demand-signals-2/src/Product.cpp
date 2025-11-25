#include "Constants.h"
#include "Product.h"
#include "Sim.h"
#include <random>

Product::Product(const std::string name, double price, int order) 
    : product_name(name), price_per_unit(price), order_size(order) {
	static std::uniform_real_distribution<> dist(PRODUCT_FREQUENCY_MIN, PRODUCT_FREQUENCY_MAX);		
	base_frequency = dist(Sim::gen);
}

int Product::get_required_labor() {
    return 1;
}

void Product::add_input(Product & input, int quantity) {
    inputs.push_back(std::unordered_map<Product&, int>({{input, quantity}}));
}