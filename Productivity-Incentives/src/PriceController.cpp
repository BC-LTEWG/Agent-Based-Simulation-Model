#include "../include/PriceController.h"
#include "../include/Firm.h" // Need this for Project struct
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>

PriceController::PriceController(std::map<std::string, double> initialOfficialPrices, std::map<std::string, std::string> units) : unit(units), gen(std::random_device{}()), reductionDist(0.05, 0.02) {
    // Copy initial official prices
    official_prices = initialOfficialPrices;
    
    // Initialize current_prices with official prices
    current_prices = initialOfficialPrices;
    
    std::cout << "Initialized official prices for " << official_prices.size() << " products\n";
}


// method to update for SimulationB
void PriceController::updateCurrentPriceForSimulationB(const std::string& productName, double newCost) {
    double oldCost = current_prices[productName];
    current_prices[productName] = newCost;
    // Debug output removed for speed
}

double PriceController::getOfficialPrice(const std::string& productName) {
    auto it = official_prices.find(productName);
    return (it != official_prices.end()) ? it->second : 0.0;
}

double PriceController::getCurrentCost(const std::string& productName) {
    auto it = current_prices.find(productName);
    if (it != current_prices.end()) {
        // Debug output removed for speed
        return it->second;
    }
    // If no actual cost data, return the official price (initial assumption)
    double officialPrice = getOfficialPrice(productName);
    // Debug output removed for speed
    return officialPrice;
}

void PriceController::updateCurrentPrice() {
    // This function should be called with the accumulated innovation rates
    // For now, just do a small reduction to avoid being too aggressive
    double price_reduction = std::clamp(reductionDist(gen), 0.005, 0.02); // Reduced: 0.5% to 2%
    for (auto& [product, price] : current_prices) {
        price = price * (1.0 - price_reduction);
    }
}


// Instance methods for compatibility
double PriceController::getAvgPriceOfProduct(const Project& project) {
    return getCurrentCost(project.productName);
}


