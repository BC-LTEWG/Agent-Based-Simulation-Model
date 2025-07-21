#pragma once
#include <map>
#include <string>
#include <vector>

// Forward declaration
struct Project;

class PriceController {
    public:
        void initializeOfficialPrices();
        void updateCurrentCosts(const std::vector<Project>& allProjects);
        void updateOfficialPrices(double thresholdPercentage);
        double getOfficialPrice(const std::string& productName);
        double getCurrentCost(const std::string& productName);
        void recomputeAverageCosts(const std::vector<Project>& allProjects);
        
        // Instance methods for compatibility with existing code
        double getAvgPriceOfProject(const Project& project);
        void updateAvgPriceOfProject(const Project& project);

    private:
        std::map<std::string, double> official_prices;
        std::map<std::string, double> current_costs;
}; 