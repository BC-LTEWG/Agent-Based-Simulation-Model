#include <cmath>
#include <numeric>

#include "Constants.h"
#include "Society.h"

Society * Society::instance = nullptr;

Society::Society() {
	instance = this;
	for (int i = 0; i < STARTING_PRODUCTS; i++) {
		products.push_back(new Product("Product " + std::to_string(i), 0, 1));
	}
	// note: no way to assigning products to producers or suppliers to distributors yet
	for (int i = 0; i < STARTING_PRODUCERS; i++) {
		Producer * producer = new Producer();
		producers.push_back(producer);
		firms.push_back(producer);
	}
	for (int i = 0; i < STARTING_DISTRIBUTORS; i++) {
		Distributor * distributor = new Distributor();
		distributors.push_back(distributor);
		firms.push_back(distributor);
	}
	// people MUST come after products and distributors are created
	for (int i = 0; i < STARTING_PEOPLE; i++) {
		birth_person();	
	}
}

std::size_t Society::num_people() {
	return people.size();
}

std::size_t Society::num_firms() {
    return firms.size();
}

bool Society::meets_standard_for_lower_working_hours() {
  double sum = 0.0;
  for (Firm * firm : firms) {
    sum += firm->get_avg_productivity();
  }
  double avg_productivity = sum / firms.size();
  return avg_productivity >= PRODUCTIVITY_THRESHOLD;
}

void Society::set_work_hours_daily(int hours) {
    current_work_hours_daily = hours;
}

double Society::get_avg_productivity() {
    return 0.0;
}

std::unordered_map<std::string, int> Society::avg_worker_needs() {
    return std::unordered_map<std::string, int>();
}

Person * Society::birth_person() {
	Person * person = new Person();
	people.push_back(person);
	add_unemployed(person);
	return person;
}

void Society::retire_person(Person * person) {
	// unimplemented until hiring/reallocation is done
}

void Society::add_unemployed(Person * person) {
	unemployed_people.push_back(person);
}
