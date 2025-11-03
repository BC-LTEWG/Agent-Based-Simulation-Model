#include <algorithm>
#include <cstdlib>
#include <iostream> // testing main directive
#include <random>

#include "Person.h"
#include "Sim.h"

Person::Person(
    const std::unordered_map<std::string, int>& expertise,
    int age,
    HealthStatus health_status,
    const std::unordered_map<Product*, double>& purchase_frequencies) :
    expertise(expertise),
    age(age),
    health_status(health_status),
	purchase_frequencies(purchase_frequencies)
{
	static std::normal_distribution<> dist(1, 1);
	for (std::pair<Product*, double> p : purchase_frequencies) {
		p.second = p.first->base_frequency * dist(Sim::gen);	
	}
}

void Person::get_paid(double income) {
    account += income;
}

void Person::charge(double cost) {
    account -= cost; 
}

std::unordered_map<std::string, double>& Person::get_worker_needs() {
    return this->needs;
}

Person::HealthStatus Person::get_health_status() {
    return this->health_status;
}

float Person::get_current_productivity() {

    static std::uniform_real_distribution<float> baseDist(0.1f, 2.0f);
    float base_productivity = baseDist(Sim::gen);
    
    switch(health_status) {
        case HEALTH_STATUS::HEALTHY:
            return std::min(base_productivity * 1.2f, 2.0f);
        case HEALTH_STATUS::RECOVERING:
            return base_productivity * 0.8f;
        case HEALTH_STATUS::UNHEALTHY:
            return base_productivity * 0.5f;
        default:
            return base_productivity;
    }
}


float Person::avg_productivity_over_time_step(std::string product_name) {
    (void)product_name;
    return 0.0f;
}

void Person::purchase_good(Product& p, int quantity) {
    (void)p;
    (void)quantity;
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

	if (age >= Society::guaranteedRetirementAge) {
		return true;
	}

	return dist(Sim::gen) < Society::randomRetirementChance;
}

int main() {
	Product product("Bread", 1, 1); 
	Person sal( {{"Computer Science", 5}}, 0, HEALTH_STATUS::HEALTHY, { {&product, 0} } );
	std::vector<Person*> workers = { &sal };
	std::vector<Firm*> firms = { new Firm() };
	Society society(workers, firms, { {firms[0], 10.0} });
	Sim sim(&society);
	std::cout << product.base_frequency << std::endl;
	return EXIT_SUCCESS;
}
		
