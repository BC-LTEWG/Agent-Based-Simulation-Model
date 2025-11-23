#include "Constants.h"
#include "Sim.h"

std::random_device Sim::rd;

std::mt19937 Sim::gen(Sim::rd());

Sim::Sim() {
	std::vector<Product*> products;
	for (int i = 0; i < STARTING_PRODUCTS; i++) {
		products.push_back(new Product("Product " + std::to_string(i), 0, 1));
	}
	std::vector<Person*> people;
	for (int i = 0; i < STARTING_PEOPLE; i++) {
		people.push_back(new Person(0, Person::HEALTHY));
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

int Sim::get_current_time_step() {
	return current_time_step;
}

std::vector<Ability> Sim::get_all_abilities() {
	return all_abilities;
}

int Sim::current_time_step = 0;

std::vector<Ability> Sim::all_abilities = { ABILITY_1, ABILITY_2, ABILITY_3 };

int main() {
	Sim simulation;
	return EXIT_SUCCESS;
}
