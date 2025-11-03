#include "Product.h"
#include "Sim.h"

Product::Product(const std::string name, double price, int order) 
    : product_name(name), price_per_unit(price), order_size(order) {
	static std::lognormal_distribution<> dist(0, 3.2);		
	base_frequency = 1 / dist(Sim::gen);
}
