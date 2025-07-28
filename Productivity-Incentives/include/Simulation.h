#pragma once
#include "Firm.h"
#include "Worker.h" 
#include "PriceController.h"
#include <vector>
#include <memory>
#include <random>
#include <string>
#include <map>

#define NUM_FIRMS 10
#define INNOVATION_PROBABILITY 0.15  // 15% chance per cycle
#define DISCOVERY_PROBABILITY 0.25   // 25% chance to discover others' innovations
#define THRESHOLD_PERCENTAGE_FIRMS 20.0    // 20% improvement needed
#define THRESHOLD_PERCENTAGE_PRODUCTS 20.0   // 20% improvement needed

class Simulation {
private:
    std::vector<std::shared_ptr<Firm>> firms;
    std::vector<Project> allProjects;
    std::mt19937 gen;
    std::uniform_real_distribution<> probability;
    std::uniform_int_distribution<> firmSelector;
    std::uniform_int_distribution<> projectSelector;
    int nextProjectId;
    int nextFirmId;
    
    // Products vector must be declared before PriceController for initialization
    std::vector<std::string> products = {"shirts", "shoes", "shorts", "apples", "bread", "chairs", "tables"};
    PriceController priceController;
    
    // Simulation parameters
    const double innovation_probability = INNOVATION_PROBABILITY;  // 15% chance per cycle
    const double discovery_probability = DISCOVERY_PROBABILITY;   // 25% chance to discover others' innovations
    const double threshold_percentage_firms = THRESHOLD_PERCENTAGE_FIRMS;   // 20% improvement needed
    const double threshold_percentage_products = THRESHOLD_PERCENTAGE_PRODUCTS;   // 20% improvement needed
    
    // Price tracking for plotting and summary
    std::map<std::string, double> initialPrices;
    std::map<std::string, std::vector<double>> priceHistory; // product -> price per cycle
    std::vector<double> workDayHours; // average work day hours per cycle
    std::vector<int> cycleNumbers; // cycle numbers for x-axis
    std::vector<std::string> firmNames = {"Alpha Corp", "Beta Industries", "Gamma Works", "Delta Manufacturing", 
                                         "Epsilon Enterprises", "Zeta Production", "Eta Systems", "Theta Co",
                                         "Iota Labs", "Kappa Industries"};

public:
    Simulation();
    
    void createFirms();
    void setProjectsPhase();
    void innovationDiscoveryPhase();
    void randomInnovationPhase();
    void productionPhase();
    void priceControllerPhase();
    void runCycle(int cycleNumber);
    void showSummary();
    void run(int numCycles = 5);
    
    // New methods for tracking and plotting
    void saveInitialPrices();
    void trackPricesAndWorkDay(int cycle);
    void generatePlots();
    double calculateAverageWorkDay();
}; 
