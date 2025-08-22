#pragma once
#include <map>
#include <string>
#include <vector>
#include <random>




// Forward declaration
struct Project;

class PriceController {
    public:
        PriceController(std::map<std::string, double> initialOfficialPrices, std::map<std::string, std::string> units);
        


        double getOfficialPrice(const std::string& productName);
        double getCurrentCost(const std::string& productName);
        void updateCurrentPrice();
        void updateCurrentPriceForSimulationB(const std::string& productName, double newCost);
        // Instance methods for compatibility with existing code
        double getAvgPriceOfProduct(const Project& project);
        
 


    private:
        std::map<std::string, double> official_prices;
        std::map<std::string, double> current_prices;
        std::map<std::string, std::string> unit;
        std::mt19937 gen;
        std::normal_distribution<double> reductionDist;
        

        


}; 