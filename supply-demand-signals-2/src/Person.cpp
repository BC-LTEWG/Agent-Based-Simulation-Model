#include "Person.h"
#include <random>
#include <algorithm>

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
  

float Person::getCurrentProductivity() {
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
