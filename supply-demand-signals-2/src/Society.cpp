#include <cmath>
#include <numeric>

#include "Constants.h"
#include "Society.h"

Society::Society(std::vector<Person*> people, std::vector<Product*> products, std::vector<Producer*> producers, std::vector<Distributor*> distributors, std::unordered_map<Product*, std::vector<Distributor*>> product_to_distributors, std::unordered_map<Firm*, double> prices)
	: people(people), products(products), unemployed_people(people), producers(producers), distributors(distributors), product_to_distributors(product_to_distributors), prices(prices) {
	for (Producer * producer : producers) {
		firms.push_back(producer);
	}
	for (Distributor * distributor : distributors) {
		firms.push_back(distributor);
	}
	for (Person * person : people) {
		person->set_society(this);
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
	Person * person = new Person({}, 0, Person::HEALTHY);
	people.push_back(person);
	unemployed_people.push_back(person);
	person->set_society(this);
	return person;
}

void Society::retire_person(Person * person) {
	// unimplemented until hiring/reallocation is done
}
