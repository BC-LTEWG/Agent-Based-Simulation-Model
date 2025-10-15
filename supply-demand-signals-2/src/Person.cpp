#include "Person.h"
#include <random>
#include <algorithm>

float Person::getCurrentProductivity() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    // Base distribution: uniform between 0.1 and 2.0 (0 exclusive, max 2x productivity)
    std::uniform_real_distribution<float> baseDist(0.1f, 2.0f);
    
    float baseProductivity = baseDist(gen);
    
    // Skew based on health status
    switch(healthStatus) {
        case HEALTH_STATUS::HEALTHY:
            // Healthy workers get a boost (multiply by 1.2, but cap at 2.0)
            return std::min(baseProductivity * 1.2f, 2.0f);
            
        case HEALTH_STATUS::RECOVERING:
            // Recovering workers get slight penalty (multiply by 0.8)
            return baseProductivity * 0.8f;
            
        case HEALTH_STATUS::UNHEALTHY:
            // Unhealthy workers get significant penalty (multiply by 0.5)
            return baseProductivity * 0.5f;
            
        default:
            return baseProductivity;
    }
}
