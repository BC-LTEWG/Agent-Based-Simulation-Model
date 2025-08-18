#pragma once
#include "Firm.h"
#include "Worker.h" 
#include "PriceController.h"
#include <vector>
#include <memory>
#include <random>
#include <string>
#include <map>

#define NUM_FIRMS 20

// Source: Toyota Production System Data (250,000 suggestions/year, 43,000 employees)
#define TOYOTA_SUGGESTIONS_PER_EMPLOYEE_YEAR 5.8    // 250,000 ÷ 43,000 = 5.8
#define TOYOTA_PARTICIPATION_RATE 0.70              // 70% of workforce participates
#define TOYOTA_IMPLEMENTATION_RATE 0.76             // 76% of suggestions implemented

// Converted to simulation cycles (assuming 1 cycle = 1 quarter)
#define EMPLOYEE_SUGGESTION_RATE 0.145              // 5.8/year ÷ 4 quarters = 1.45 per quarter ≈ 14.5% chance
#define SUGGESTION_IMPLEMENTATION_RATE 0.76         // 76% implementation rate (Toyota data)
#define WORKER_PARTICIPATION_RATE 0.70              // 70% participate (Toyota data)

// Productivity impact from academic research
#define EMPLOYEE_INNOVATION_PRODUCTIVITY_GAIN 0.212 // 21.2% productivity increase (research study)
#define PATENT_RETENTION_EFFECT 0.23                // 23% reduced turnover for innovators

// Innovation distribution (research-based)
#define UNPATENTED_INNOVATION_RATE 0.561            // 56.1% of innovations unpatented (Swedish study)
#define HIGH_INNOVATOR_PERCENTAGE 0.20              // 20% of workers generate 80% of innovations
#define INNOVATION_DISCOVERY_RATE 0.25              // Inter-firm knowledge transfer rate

// Economic thresholds for price updates
#define THRESHOLD_PERCENTAGE_FIRMS 12.0      // 12% improvement needed (research-based)
#define THRESHOLD_PERCENTAGE_PRODUCTS 20.0   // 20% of products must improve consistently

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
    
    // Research-based simulation parameters using concrete Toyota and academic data
    const double employee_suggestion_rate = EMPLOYEE_SUGGESTION_RATE;     // 14.5% per worker (Toyota: 5.8/year ÷ 4 quarters)
    const double innovation_discovery_rate = INNOVATION_DISCOVERY_RATE;   // 25% chance to discover others
    const double worker_participation_rate = WORKER_PARTICIPATION_RATE;   // 70% participation (Toyota data)
    const double suggestion_implementation_rate = SUGGESTION_IMPLEMENTATION_RATE; // 76% implementation (Toyota data)
    const double threshold_percentage_firms = THRESHOLD_PERCENTAGE_FIRMS;     // 12% improvement needed
    const double threshold_percentage_products = THRESHOLD_PERCENTAGE_PRODUCTS; // 20% of products must improve
    
    // Price tracking for plotting and summary
    std::map<std::string, double> initialPrices;
    std::map<std::string, std::vector<double>> priceHistory; // product -> price per cycle
    std::vector<double> workDayHours; // average work day hours per cycle
    std::vector<int> cycleNumbers; // cycle numbers for x-axis
    std::vector<std::string> firmNames = {"Alpha Corp", "Beta Industries", "Gamma Works", "Delta Manufacturing", 
                                         "Epsilon Enterprises", "Zeta Production", "Eta Systems", "Theta Co",
                                         "Iota Labs", "Kappa Industries", "Lambda Services", "Mu Healthcare",
                                         "Nu Logistics", "Xi Technologies", "Omicron Foods", "Pi Construction",
                                         "Rho Textiles", "Sigma Energy", "Tau Transport", "Upsilon Care"};
    
    // Economic shock tracking
    std::vector<std::pair<int, std::string>> economicShockHistory; // cycle, shock type
    double shockProbabilityPerCycle = 0.15; // 15% chance per cycle
    int cyclesSinceLastShock = 0;
    
    // Productivity tracking by sector
    std::map<std::string, double> sectorProductivityGains; // sector -> total productivity gain
    std::map<std::string, std::vector<std::string>> productSectors; // sector -> products
    std::map<std::string, double> sectorLaborIntensity; // sector -> labor intensity factor

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
    
    // Economic shock methods
    void triggerEconomicShock(int cycleNumber);
    bool shouldTriggerShock(int cycleNumber);
    void applyProductivityShock(double severity);
    void applyDemandShock(double severity);
    void applySupplyChainShock(double severity);
    
    // New plotting methods
    void generateProductWorkdayPlot(const std::string& productName);
    void generateAveragePriceWorkdayPlot();
    void generateProductivityPieChart();
    void generateProductivityPieChartForCycle(int cycle);
    void categorizeProductsByLaborIntensity();
}; 
