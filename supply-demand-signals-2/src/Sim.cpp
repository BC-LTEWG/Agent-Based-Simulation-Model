#include "Constants.h"
#include "Sim.h"

std::random_device Sim::rd;

std::mt19937 Sim::gen(Sim::rd());

Sim::Sim() : current_time_step(0) {
	std::vector<Product*> products;
	for (int i = 0; i < STARTING_PRODUCTS; i++) {
		products.push_back(new Product("Product " + std::to_string(i), 0, 1));
	}
	std::vector<Person*> people;
	for (int i = 0; i < STARTING_PEOPLE; i++) {
		std::unordered_map<Product*, double> purchase_frequencies;
		for (Product* p : products) {
			purchase_frequencies[p] = 0.0;
		}
		people.push_back(new Person({}, 0, HEALTH_STATUS::HEALTHY, purchase_frequencies));
	}
	// note: no way to assing products to producers or suppliers to distributors yet
	std::vector<Producer*> producers;
	for (int i = 0; i < STARTING_PRODUCERS; i++) {
		producers.push_back(new Producer());
	}
	std::vector<Distributor*> distributors;
	for (int i = 0; i < STARTING_DISTRIBUTORS; i++) {
		distributors.push_back(new Distributor());
	}
	society = new Society(people, products, producers, distributors, {}, {});	
}

int main() {
	Sim simulation;
	return EXIT_SUCCESS;
}
