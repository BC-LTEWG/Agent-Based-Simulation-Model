#include "../include/PriceController.h"
#include "../include/Firm.h" // Need this for Project struct
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>

PriceController::PriceController(std::vector<std::string> products, std::vector<int> prices) : products(products), prices(prices) {
    // Initialize official prices using for loop with products and prices vectors
    for (size_t i = 0; i < products.size() && i < prices.size(); ++i) {
        official_prices[products[i]] = static_cast<double>(prices[i]);
    }
    
    current_costs = official_prices;
    
    std::cout << "Initialized official prices for " << official_prices.size() << " products\n";
}


void PriceController::updateCurrentCosts(const std::vector<Project>& allProjects) {
    recomputeAverageCosts(allProjects);
}

void PriceController::updateOfficialPrices(double thresholdPercentageFirms, double thresholdPercentageProducts) {
   
    int numFirmPriceChanges = 0;
    for (auto & [product, currentCost] : current_costs) {
        if (official_prices.find(product) != official_prices.end()) {
            double officialPrice = official_prices[product];
            double percentageDiff = ((officialPrice - currentCost) / officialPrice) * 100.0;
            if (percentageDiff >= thresholdPercentageFirms) {
                numFirmPriceChanges++;
            }

        }
    }

    if (((numFirmPriceChanges / products.size()) * 100.0) >= thresholdPercentageProducts) {
        for (auto & [product, currentCost] : current_costs) {
            official_prices[product] = currentCost;
        }
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
