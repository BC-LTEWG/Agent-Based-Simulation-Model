#include <algorithm>
#include <cstdlib>
#include <iostream> // testing main directive
#include <random>

#include "Constants.h"
#include "Person.h"
#include "Sim.h"

Person::Person(
    const std::unordered_map<std::string, int>& expertise,
    int age,
    HealthStatus health_status) :
    expertise(expertise),
    age(age),
    health_status(health_status)
{
}

void Person::get_paid(double income) {
    account += income;
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

void Person::purchase_goods() {
	static std::uniform_real_distribution<> dist(0, 1);

	for (std::pair<Product*, double> p : purchase_frequencies) {
		if (dist(Sim::gen) < p.second) {
			purchase_good(p.first, 1);
		}
	}
}

bool Person::will_retire() {
	static std::uniform_real_distribution<> dist(0, 1);

	if (age >= GUARANTEED_RETIREMENT_AGE) {
		return true;
	}

	return dist(Sim::gen) < RANDOM_RETIREMENT_CHANCE;
}

void Person::set_society(Society * society) {
	this->society = society;
	ranked_distributors = society->distributors;
	std::shuffle(ranked_distributors.begin(), ranked_distributors.end(), Sim::gen);
	static std::normal_distribution<> dist(PERSON_FREQUENCY_MULTIPLIER_MEAN, PERSON_FREQUENCY_MULTIPLIER_STDDEV);
	for (Product * p : society->products) {
		purchase_frequencies[p] = p->base_frequency * dist(Sim::gen);
	}
}
		
