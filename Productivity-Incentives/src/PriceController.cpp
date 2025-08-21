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


double PriceController::getOfficialPrice(const std::string& productName) {
    auto it = official_prices.find(productName);
    return (it != official_prices.end()) ? it->second : 0.0;
}

double PriceController::getCurrentCost(const std::string& productName) {
    auto it = current_prices.find(productName);
    if (it != current_prices.end()) {
        return it->second;
    }
    // If no actual cost data, return the official price (initial assumption)
    return getOfficialPrice(productName);
}

void PriceController::updateCurrentPrice() {
    double price_reduction = std::clamp(reductionDist(gen), 0.01, 0.10);
    for (auto& [product, price] : current_prices) {
        price = price * (1.0 - price_reduction);
    }
}


// Instance methods for compatibility
double PriceController::getAvgPriceOfProduct(const Project& project) {
    return getCurrentCost(project.productName);
}


