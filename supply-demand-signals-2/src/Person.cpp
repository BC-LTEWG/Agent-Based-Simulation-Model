#include <algorithm>
#include <cstdlib>
#include <iostream> // testing main directive
#include <random>

#include "Person.h"
#include "Sim.h"

Person::Person():
    age(INITIAL_AGE),
    health_status(HEALTHY)
{
	static std::normal_distribution<> shopping_dist(PERSON_SHOPPING_PERIOD / 2, PERSON_SHOPPING_OFFSET_STDDEV);
	shopping_offset = (((int) shopping_dist(Sim::gen)) + PERSON_SHOPPING_PERIOD) % PERSON_SHOPPING_PERIOD;

	static std::normal_distribution<> ability_dist(1.0, PERSON_ABILITY_STDDEV);
	std::vector<Ability> all_abilities;
	for (int i = 0; i < NUM_ABILITIES; i++) {
		all_abilities.push_back((Ability) i);
	}
	std::shuffle(all_abilities.begin(), all_abilities.end(), Sim::gen);
	all_abilities.resize(PERSON_ABILITY_COUNT_MAX);
	for (Ability ability : all_abilities) {
		abilities[ability] = std::max(0.0, ability_dist(Sim::gen));
	}
	ranked_distributors = Society::instance->distributors;
	std::shuffle(ranked_distributors.begin(), ranked_distributors.end(), Sim::gen);
	static std::normal_distribution<> dist(1, PERSON_FREQUENCY_MULTIPLIER_STDDEV);
	for (Product * p : Society::instance->products) {
		purchase_frequencies[p] = p->base_frequency * std::abs(dist(Sim::gen));
	}
}

void Person::register_hours_worked(double hours_worked) {
    account += hours_worked;
}

void Person::charge(double cost) {
    account -= cost; 
}

std::unordered_map<Product*, double>& Person::get_purchase_frequencies() { 
	return this->purchase_frequencies;
}

Person::HealthStatus Person::get_health_status() {
    return this->health_status;
}

float Person::get_current_productivity() {

    static std::uniform_real_distribution<float> baseDist(0.1f, 2.0f);
    float base_productivity = baseDist(Sim::gen);
    
    switch(health_status) {
        case HEALTHY:
            return std::min(base_productivity * 1.2f, 2.0f);
        case RECOVERING:
            return base_productivity * 0.8f;
        case UNHEALTHY:
            return base_productivity * 0.5f;
        default:
            return base_productivity;
    }
}

float Person::avg_productivity_over_time_step(std::string product_name) {
    (void)product_name;
    return 0.0f;
}

void Person::purchase_good(Product * p, int quantity) {
	for (Distributor * distributor : ranked_distributors) {
		if (distributor->has_product(p)) {
			distributor->sell_goods(*p, quantity, this);
			return;
		}
	}
}

bool Person::will_shop() {
	return Sim::get_current_time_step() % PERSON_SHOPPING_PERIOD == shopping_offset;
}

void Person::shop() {
	static std::normal_distribution<> dist(1, PERSON_SHOPPING_MULTIPLIER_STDDEV);
	for (auto &p : purchase_frequencies) {
		int quantity = std::round(p.second * PERSON_SHOPPING_PERIOD * std::abs(dist(Sim::gen)));
		if (quantity > 0) {
			purchase_good(p.first, quantity);
		}
	}
}

bool Person::will_retire() {
	static std::uniform_real_distribution<> dist(0, 1);
	if (age >= GUARANTEED_RETIREMENT_AGE) { return true; }
	return dist(Sim::gen) < RANDOM_RETIREMENT_CHANCE;
}

void Person::retire() {
	Society::instance->retire_person(this);
}

void Person::on_time_step() {
	++age;
	if (will_shop()) { shop(); }
	if (will_retire()) { retire(); }
}

void Person::set_firm(Firm * workplace) {
    firm = workplace;
}
