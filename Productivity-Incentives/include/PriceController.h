#pragma once
#include <map>
#include <string>
#include <vector>



// Forward declaration
struct Project;

class PriceController {
    public:
        PriceController(std::vector<std::string> products, std::vector<int> prices);
        
        void updateCurrentCosts(const std::vector<Project>& allProjects);
        void updateOfficialPrices(double thresholdPercentageFirms, double thresholdPercentageProducts);
        double getOfficialPrice(const std::string& productName);
        double getCurrentCost(const std::string& productName);
        void recomputeAverageCosts(const std::vector<Project>& allProjects);
        
        // Instance methods for compatibility with existing code
        double getAvgPriceOfProject(const Project& project);
        void updateAvgPriceOfProject(const Project& project);

    private:
        std::map<std::string, double> official_prices;
        std::map<std::string, double> current_costs;
        std::vector<std::string> products;
        std::vector<int> prices;
}; 