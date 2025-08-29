#include "../include/Simulation.h"
#include <thread>
#include <iostream> // Added for std::cout
#include <cstdlib> // For setenv

int main() {
    // Set environment variables to disable interactive plotting
    setenv("DISPLAY", "", 1);
    setenv("MPLBACKEND", "Agg", 1);
    
    // Run Simulation B with same parameters as Simulation A
    std::cout << "Starting Simulation B..." << std::endl;
    SimulationB sim;
    sim.run(10000); // Run 10,000 cycles
    
    std::cout << "Simulation B completed. Saving results..." << std::endl;
    sim.saveResultsToCSV();
    sim.saveSummaryStatistics();
    std::cout << "Done!" << std::endl;
    
    return 0;
}
