#include "../include/PriceController.h"
#include "../include/Firm.h" // Need this for Project struct
#include <iostream>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>


void PriceController::initializeOfficialPrices() {
    official_prices["shirts"] = 5.0;    // 5 labor hours
    official_prices["shoes"] = 8.0;     // 8 labor hours
    official_prices["shorts"] = 3.0;    // 3 labor hours
    official_prices["apples"] = 0.5;    // 0.5 labor hours
    official_prices["bread"] = 2.0;     // 2 labor hours
    official_prices["chairs"] = 12.0;   // 12 labor hours
    official_prices["tables"] = 20.0;   // 20 labor hours
    
    current_costs = official_prices;
    
    std::cout << "Initialized official prices for " << official_prices.size() << " products\n";
}

void PriceController::updateCurrentCosts(const std::vector<Project>& allProjects) {
    recomputeAverageCosts(allProjects);
}

void PriceController::updateOfficialPrices(double thresholdPercentage) {
    int updatedCount = 0;
    std::unordered_map<std::string, double> updatedPrices;
    for (const auto& [product, currentCost] : current_costs) {
        if (official_prices.find(product) != official_prices.end()) {
            double officialPrice = official_prices[product];
            double percentageDiff = ((officialPrice - currentCost) / officialPrice) * 100.0;
            
            if (percentageDiff >= thresholdPercentage) {
                updatedPrices[product] = currentCost;
                // official_prices[product] = currentCost;
                // std::cout << "Updated official price for " << product << " from " 
                //         << officialPrice << " to " << currentCost << " labor hours\n";
                updatedCount++;
            }
        }

        if (updatedCount > current_costs.size() * 0.5) {
            for (const auto& [product, costs]: current_costs) {
                official_prices[product] = costs;
            }
        }
    }
    
    if (updatedCount > 0) {
        std::cout << "Average work day is now shorter! Updated " << updatedCount << " prices.\n";
    }
}

double PriceController::getOfficialPrice(const std::string& productName) {
    auto it = official_prices.find(productName);
    return (it != official_prices.end()) ? it->second : 0.0;
}

double PriceController::getCurrentCost(const std::string& productName) {
    auto it = current_costs.find(productName);
    return (it != current_costs.end()) ? it->second : 0.0;
}

void PriceController::recomputeAverageCosts(const std::vector<Project>& allProjects) {
    std::map<std::string, std::vector<double>> productCosts;
    
    // Collect all costs for each product
    for (const auto& project : allProjects) {
        if (project.actualHoursSpent > 0 && project.productQuantity > 0) {
            double costPerUnit = project.actualHoursSpent / project.productQuantity;
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
            current_costs[product] = sum / costs.size();
        }
    }
}

// Instance methods for compatibility
double PriceController::getAvgPriceOfProject(const Project& project) {
    return getCurrentCost(project.productName);
}
