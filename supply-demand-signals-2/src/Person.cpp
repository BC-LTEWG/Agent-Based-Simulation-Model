#include <algorithm>
#include <cstdlib>
#include <random>

#include "Person.h"

Person::Person(
    const std::unordered_map<std::string, int>& expertise,
    int age,
    HealthStatus health_status,
    const std::unordered_map<std::string, double>& needs) :
    expertise(expertise),
    age(age),
    health_status(health_status),
    needs(needs)
{}

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

void Person::purchase_goods(Product& p, int quantity) {
    (void)p;
    (void)quantity;
}

bool Person::willRetire() {
	if (age >= Society.guaranteedRetirementAge) {
		return true;
	}
	return (float) std::rand() / RAND_MAX < Society.randomRetirementChance;
}
		
