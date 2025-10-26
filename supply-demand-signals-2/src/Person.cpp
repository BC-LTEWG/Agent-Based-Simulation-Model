#include "Person.h"
#include <random>
#include <algorithm>

float Person::getCurrentProductivity() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    std::uniform_real_distribution<float> baseDist(0.1f, 2.0f);
    
    float baseProductivity = baseDist(gen);
    
    switch(healthStatus) {
        case HEALTH_STATUS::HEALTHY:
            return std::min(baseProductivity * 1.2f, 2.0f);
        case HEALTH_STATUS::RECOVERING:
            return baseProductivity * 0.8f;
        case HEALTH_STATUS::UNHEALTHY:
            return baseProductivity * 0.5f;
            
        default:
            return baseProductivity;
    }
}
