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

void Product::set_leontief_matrix() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.0, 1.0);
    for(int i = 0; i < leontief_matrix.size(); i++) {
        for(int j = 0; j < leontief_matrix[i].size(); j++) {
            leontief_matrix[i][j][this] = dist(gen);
        }
    }
}
