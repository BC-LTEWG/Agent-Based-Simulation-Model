#pragma once
#include "Firm.h"
#include "Worker.h" 
#include "PriceController.h"
#include <vector>
#include <memory>
#include <random>
#include <string>

class Simulation {
private:
    std::vector<std::shared_ptr<Firm>> firms;
    std::vector<Project> allProjects;
    std::mt19937 gen;
    std::uniform_real_distribution<> probability;
    std::uniform_int_distribution<> firmSelector;
    int nextProjectId;
    int nextFirmId;
    PriceController priceController;
    
    // Simulation parameters
    const double INNOVATION_PROBABILITY = 0.15;  // 15% chance per cycle
    const double DISCOVERY_PROBABILITY = 0.25;   // 25% chance to discover others' innovations
    const double THRESHOLD_PERCENTAGE = 20.0;    // 20% improvement needed
    const double DELTA_THRESHOLD = 0.5;          // Minimum 0.5 hour improvement
    
    std::vector<std::string> products = {"shirts", "shoes", "shorts", "apples", "bread", "chairs", "tables"};
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
}; 