#include "../include/PriceController.h"
#include "../include/Firm.h" // Need this for Project struct
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>

PriceController::PriceController(std::map<std::string, double> initialOfficialPrices, std::map<std::string, std::string> units) : unit(units) {
    // Copy initial official prices
    official_prices = initialOfficialPrices;
    
    // Initialize current_prices as empty - they will be filled as projects complete
    // current_prices = official_prices;  // Don't copy - let them be computed from actual data
    
    std::cout << "Initialized official prices for " << official_prices.size() << " products\n";
}


void PriceController::updateCurrentCosts(const std::vector<Project>& allProjects) {
    recomputeAverageCosts(allProjects);
}

void PriceController::updateOfficialPrices(double thresholdPercentageFirms, double thresholdPercentageProducts) {
   
    int numFirmPriceChanges = 0;
    for (auto & [product, currentCost] : current_prices) {
        if (official_prices.find(product) != official_prices.end()) {
            double officialPrice = official_prices[product];
            double percentageDiff = ((officialPrice - currentCost) / officialPrice) * 100.0;
            if (percentageDiff >= thresholdPercentageFirms) {
                numFirmPriceChanges++;
            }

        }
    }

    double percentageOfProductsImproved = (numFirmPriceChanges / static_cast<double>(official_prices.size())) * 100.0;
    std::cout << "Price update check: " << numFirmPriceChanges << "/" << official_prices.size() 
              << " products improved (" << percentageOfProductsImproved << "%)\n";
    std::cout << "Threshold needed: " << thresholdPercentageProducts << "%\n";
    
    if (percentageOfProductsImproved >= thresholdPercentageProducts) {
        std::cout << "UPDATING OFFICIAL PRICES based on current costs:\n";
        for (auto & [product, currentCost] : current_prices) {
            double oldPrice = official_prices[product];
            official_prices[product] = currentCost;
            std::cout << "  " << product << ": " << oldPrice << " -> " << currentCost << "\n";
        }
    } else {
        std::cout << "Official prices remain unchanged.\n";
    }

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

void PriceController::recomputeAverageCosts(const std::vector<Project>& allProjects) {
    std::map<std::string, std::vector<double>> productCosts;
    
    // Collect all costs for each product
    for (const auto& project : allProjects) {
        if (project.actualCost > 0 && project.quantity > 0) {
            double costPerUnit = project.actualCost / project.quantity;
            productCosts[project.productName].push_back(costPerUnit);
        }
    }
    
    // Calculate averages
    for (const auto& [product, costs] : productCosts) {
        if (!costs.empty()) {
            double sum = 0.0;
            for (double cost : costs) {
                sum += cost;
            }
            current_prices[product] = sum / costs.size();
        }
    }
}

// Instance methods for compatibility
double PriceController::getAvgPriceOfProduct(const Project& project) {
    return getCurrentCost(project.productName);
}
