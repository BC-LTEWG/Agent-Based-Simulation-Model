#include <algorithm>
#include <cstdlib>
#include <random>

#include "Person.h"

Person::Person(
    const std::unordered_map<std::string, int>& expertise,
    int age,
    HealthStatus health_status,
    const std::unordered_map<Product*, double>& needs) :
    expertise(expertise),
    age(age),
    health_status(health_status),
    needs(needs)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
	static std::lognormal_distribution<> dist(0, 3.2);		
	for (std::pair<Product*, double> p  : needs) {
		p.second = 1 / dist(gen);
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
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    std::uniform_real_distribution<float> baseDist(0.1f, 2.0f);
    
    float baseProductivity = baseDist(gen);
    
    switch(health_status) {
        case HEALTHY:
            return std::min(baseProductivity * 1.2f, 2.0f);
        case RECOVERING:
            return baseProductivity * 0.8f;
        case UNHEALTHY:
            return baseProductivity * 0.5f;
        default:
            return baseProductivity;
    }
}


float Person::avg_productivity_over_time_step(std::string product_name) {
    (void)product_name;
    return 0.0f;
}

void Person::purchaseGood(Product& p, int quantity) {
    (void)p;
    (void)quantity;
}

void Person::purchaseGoods() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
	static std::uniform_real_distribution<> dist(0, 1);

	for (std::pair<Product*, double> p : needs) {
		if (dist(gen) < p.second) {
			purchaseGood(*p.first, 1);
		}
	}
}

bool Person::willRetire() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
	static std::uniform_real_distribution<> dist(0, 1);

	if (age >= Society::guaranteedRetirementAge) {
		return true;
	}

	return dist(gen) < Society::randomRetirementChance;
}

int main() {
	return EXIT_SUCCESS;
}
		
