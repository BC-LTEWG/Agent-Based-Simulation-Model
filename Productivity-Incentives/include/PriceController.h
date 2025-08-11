#pragma once
#include <map>
#include <string>
#include <vector>



// Forward declaration
struct Project;

class PriceController {
    public:
        PriceController(std::map<std::string, double> initialOfficialPrices, std::map<std::string, std::string> units);
        
        void updateCurrentCosts(const std::vector<Project>& allProjects);
        void updateOfficialPrices(double thresholdPercentageFirms, double thresholdPercentageProducts);
        double getOfficialPrice(const std::string& productName);
        double getCurrentCost(const std::string& productName);
        void recomputeAverageCosts(const std::vector<Project>& allProjects);
        
        // Instance methods for compatibility with existing code
        double getAvgPriceOfProduct(const Project& project);
        void updateAvgPriceOfProject(const Project& project);
        
        // New methods for fixed productivity threshold
        void resetProductivityCounters();
        bool hasMetFixedThreshold(const std::string& productName, double fixedThreshold);
        void trackProductivityImprovements(double fixedThreshold);

    private:
        std::map<std::string, double> official_prices;
        std::map<std::string, double> current_prices;
        std::map<std::string, std::string> unit;
        
        // Fixed productivity threshold tracking
        std::map<std::string, int> productivity_improvement_cycles; // Track consecutive cycles of improvement
        std::map<std::string, double> best_productivity_achieved;   // Track best productivity per product
        static const int REQUIRED_CONSECUTIVE_CYCLES = 3;          // Require 3 consecutive cycles of improvement

}; 