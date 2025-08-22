#include "../include/Simulation.h"
#include <thread>
#include <iostream> // Added for std::cout

int main() {
    // Run only Simulation B for debugging
    std::cout << "Starting Simulation B (debug mode)..." << std::endl;
    SimulationB simb;
    simb.run(10000);
    
    std::cout << "Simulation B completed. Saving results..." << std::endl;
    simb.saveResultsToCSV();
    simb.saveSummaryStatistics();
    std::cout << "Done!" << std::endl;
    
    return 0;
}
