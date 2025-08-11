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
    // Track productivity improvements for fixed threshold
    trackProductivityImprovements(thresholdPercentageFirms);
    
    int numFirmPriceChanges = 0;
    int numProductsMetFixedThreshold = 0;
    
    for (auto & [product, currentCost] : current_prices) {
        if (official_prices.find(product) != official_prices.end()) {
            double officialPrice = official_prices[product];
            double percentageDiff = ((officialPrice - currentCost) / officialPrice) * 100.0;
            
            // Check immediate threshold
            if (percentageDiff >= thresholdPercentageFirms) {
                numFirmPriceChanges++;
            }
            
            // Check fixed threshold (consecutive cycles)
            if (hasMetFixedThreshold(product, thresholdPercentageFirms)) {
                numProductsMetFixedThreshold++;
            }
        }
    }

    double percentageOfProductsImproved = (numFirmPriceChanges / static_cast<double>(official_prices.size())) * 100.0;
    double percentageMetFixedThreshold = (numProductsMetFixedThreshold / static_cast<double>(official_prices.size())) * 100.0;
    
    std::cout << "Price update check:\n";
    std::cout << "  Immediate improvements: " << numFirmPriceChanges << "/" << official_prices.size() 
              << " products (" << percentageOfProductsImproved << "%)\n";
    std::cout << "  Fixed threshold met: " << numProductsMetFixedThreshold << "/" << official_prices.size() 
              << " products (" << percentageMetFixedThreshold << "%)\n";
    std::cout << "  Threshold needed: " << thresholdPercentageProducts << "%\n";
    
    // Use fixed threshold for more stable price updates
    if (percentageMetFixedThreshold >= thresholdPercentageProducts) {
        std::cout << "UPDATING OFFICIAL PRICES based on fixed productivity threshold:\n";
        for (auto & [product, currentCost] : current_prices) {
            if (hasMetFixedThreshold(product, thresholdPercentageFirms)) {
                double oldPrice = official_prices[product];
                official_prices[product] = currentCost;
                std::cout << "  " << product << ": " << oldPrice << " -> " << currentCost 
                         << " (sustained improvement)\n";
            }
        }
        // Reset counters after successful update
        resetProductivityCounters();
    } else {
        std::cout << "Official prices remain unchanged (fixed threshold not met).\n";
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

// Fixed productivity threshold methods
void PriceController::resetProductivityCounters() {
    productivity_improvement_cycles.clear();
    best_productivity_achieved.clear();
    std::cout << "Productivity counters reset after official price update.\n";
}

bool PriceController::hasMetFixedThreshold(const std::string& productName, double /*fixedThreshold*/) {
    auto cycleIt = productivity_improvement_cycles.find(productName);
    if (cycleIt == productivity_improvement_cycles.end()) {
        return false;
    }
    
    // Check if this product has met the threshold for required consecutive cycles
    return cycleIt->second >= REQUIRED_CONSECUTIVE_CYCLES;
}

void PriceController::trackProductivityImprovements(double fixedThreshold) {
    for (auto & [product, currentCost] : current_prices) {
        if (official_prices.find(product) != official_prices.end()) {
            double officialPrice = official_prices[product];
            double percentageDiff = ((officialPrice - currentCost) / officialPrice) * 100.0;
            
            // Check if this cycle shows improvement
            if (percentageDiff >= fixedThreshold) {
                // Track best productivity achieved
                auto bestIt = best_productivity_achieved.find(product);
                if (bestIt == best_productivity_achieved.end() || currentCost < bestIt->second) {
                    best_productivity_achieved[product] = currentCost;
                }
                
                // Increment consecutive improvement cycles
                productivity_improvement_cycles[product]++;
                std::cout << "  " << product << ": improvement cycle " 
                         << productivity_improvement_cycles[product] << "/" 
                         << REQUIRED_CONSECUTIVE_CYCLES << " (savings: " 
                         << percentageDiff << "%)\n";
            } else {
                // Reset counter if improvement is not sustained
                if (productivity_improvement_cycles.find(product) != productivity_improvement_cycles.end()) {
                    std::cout << "  " << product << ": improvement cycle reset (insufficient improvement)\n";
                    productivity_improvement_cycles[product] = 0;
                }
            }
        }
    }
}
