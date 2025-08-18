#include "../include/Simulation.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <matplot/matplot.h>

Simulation::Simulation() : gen(std::random_device{}()), probability(0.0, 1.0), nextProjectId(1), nextFirmId(1),
    priceController({{"shirts", 5.0}, {"shoes", 8.0}, {"shorts", 3.0}, {"apples", 1.0}, {"bread", 2.0}, {"chairs", 12.0}, {"tables", 20.0}}, 
                    {{"shirts", "units"}, {"shoes", "pairs"}, {"shorts", "units"}, {"apples", "kg"}, {"bread", "loaves"}, {"chairs", "units"}, {"tables", "units"}}) {
    saveInitialPrices();
    categorizeProductsByLaborIntensity();
    createFirms();
}

void Simulation::saveInitialPrices() {
    for (const auto& product : products) {
        initialPrices[product] = priceController.getOfficialPrice(product);
        priceHistory[product] = std::vector<double>();
    }
}

double Simulation::calculateAverageWorkDay() {
    // Work day calculation with diminishing returns and saturation
    double baseWorkDay = 8.0; // hours
    static double minimumWorkDayAchieved = baseWorkDay;
    static double cumulativeProductivityGain = 0.0; // Track cumulative improvements
    static int totalCyclesWithImprovement = 0;
    
    // Define saturation parameters
    const double MINIMUM_WORKDAY = 4.0; // Cannot go below 4 hours (50% of original)
    const double DIMINISHING_FACTOR = 0.8; // Each cycle of improvement has 80% of previous effect
    const double SATURATION_THRESHOLD = 0.9; // 90% of maximum possible improvement
    
    double totalProductivityGain = 0.0;
    int productCount = 0;
    
    // Only count products that have demonstrable improvements from innovation
    for (const auto& product : products) {
        double officialPrice = priceController.getOfficialPrice(product);
        double actualAverage = priceController.getCurrentCost(product);
        
        if (officialPrice > 0 && actualAverage > 0 && actualAverage < officialPrice) {
            double improvement = (officialPrice - actualAverage) / officialPrice;
            totalProductivityGain += improvement;
            productCount++;
            
            std::cout << "Product " << product << ": official=" << officialPrice 
                      << ", actual=" << actualAverage << ", improvement=" << (improvement * 100.0) << "%\n";
        }
    }
    
    double newWorkDay;
    if (productCount == 0) {
        // No improvements detected, keep current minimum
        newWorkDay = minimumWorkDayAchieved;
        std::cout << "Work day calculation: No productivity improvements, maintaining " << newWorkDay << " hours\n";
    } else {
        // Apply diminishing returns to productivity gains
        double avgProductivityGain = totalProductivityGain / productCount;
        
        // Apply diminishing returns based on previous improvements
        double effectiveGain = avgProductivityGain * std::pow(DIMINISHING_FACTOR, totalCyclesWithImprovement);
        
        // Add to cumulative gains
        cumulativeProductivityGain += effectiveGain;
        totalCyclesWithImprovement++;
        
        // Calculate maximum possible improvement (baseWorkDay - MINIMUM_WORKDAY)
        double maxPossibleImprovement = (baseWorkDay - MINIMUM_WORKDAY) / baseWorkDay;
        
        // Check for saturation
        if (cumulativeProductivityGain >= maxPossibleImprovement * SATURATION_THRESHOLD) {
            std::cout << "Work day approaching saturation limit!\n";
            cumulativeProductivityGain = maxPossibleImprovement * SATURATION_THRESHOLD;
        }
        
        // Calculate new work day with saturation limit
        newWorkDay = baseWorkDay * (1.0 - cumulativeProductivityGain);
        
        // Enforce minimum work day limit
        if (newWorkDay < MINIMUM_WORKDAY) {
            newWorkDay = MINIMUM_WORKDAY;
            std::cout << "Work day has reached minimum threshold of " << MINIMUM_WORKDAY << " hours\n";
        }
        
        // Work day can only decrease (ratchet effect)
        if (newWorkDay < minimumWorkDayAchieved) {
            minimumWorkDayAchieved = newWorkDay;
        } else {
            newWorkDay = minimumWorkDayAchieved;
        }
        
        std::cout << "Work day calculation: products improved=" << productCount 
                  << ", rawGain=" << (avgProductivityGain * 100.0) 
                  << "%, effectiveGain=" << (effectiveGain * 100.0)
                  << "%, cumulativeGain=" << (cumulativeProductivityGain * 100.0)
                  << "%, workDay=" << newWorkDay << " hours\n";
        std::cout << "  Diminishing factor applied: " << std::pow(DIMINISHING_FACTOR, totalCyclesWithImprovement - 1) 
                  << " (cycle " << totalCyclesWithImprovement << ")\n";
        
        // Show saturation progress
        double saturationProgress = (cumulativeProductivityGain / (maxPossibleImprovement * SATURATION_THRESHOLD)) * 100.0;
        std::cout << "  Saturation progress: " << saturationProgress << "%\n";
    }
    
    return newWorkDay;
}

void Simulation::trackPricesAndWorkDay(int cycle) {
    cycleNumbers.push_back(cycle);
    
    // Track current prices for each product
    for (const auto& product : products) {
        priceHistory[product].push_back(priceController.getCurrentCost(product));
    }
    
    // Track average work day hours
    workDayHours.push_back(calculateAverageWorkDay());
}

void Simulation::generatePlots() {
    using namespace matplot;
    
    // Convert cycle numbers to double for plotting
    std::vector<double> cycles;
    for (int cycle : cycleNumbers) {
        cycles.push_back(static_cast<double>(cycle));
    }
    
    // Create figure with subplots
    auto fig = figure(true);
    fig->size(1200, 800);
    
    // First subplot: Commodity prices
    subplot(2, 1, 1);
    
    // Plot each product price history
    for (const auto& product : products) {
        const auto& prices = priceHistory[product];
        auto p = plot(cycles, prices, "-s");
        p->color("red");
        p->display_name(product);
        p->line_width(2);
        xlabel("Cycle");
        ylabel("Labor Hours per Unit");
        title("Commodity Prices Over Time");
        legend();
        grid(on);
    }    
    
    auto p2 = plot(cycles, workDayHours, "-s");
    p2->color("red");
    p2->line_width(2);
    
    xlabel("Cycle");
    ylabel("Average Work Day (Hours)");
    title("Average Work Day Duration Over Time");
    grid(on);
    
    // Save and show the plot
    save("simulation_results.png");
    show();
    
    // Generate CSV file for external analysis
    std::ofstream csvFile("simulation_data.csv");
    csvFile << "Cycle,";
    for (const auto& product : products) {
        csvFile << product << "_price,";
    }
    csvFile << "work_day_hours\n";
    
    for (size_t i = 0; i < cycleNumbers.size(); ++i) {
        csvFile << cycleNumbers[i] << ",";
        for (const auto& product : products) {
            csvFile << std::fixed << std::setprecision(4) << priceHistory[product][i] << ",";
        }
        csvFile << std::fixed << std::setprecision(4) << workDayHours[i] << "\n";
    }
    csvFile.close();
    
    // Create simple ASCII visualization for work day trends
    std::cout << "\n=== WORK DAY TREND (ASCII) ===\n";
    std::cout << "Cycle | Work Day Hours | Trend\n";
    std::cout << "------|-----------------|------\n";
    
    for (size_t i = 0; i < cycleNumbers.size(); ++i) {
        double hours = workDayHours[i];
        int barLength = static_cast<int>((hours / 8.0) * 20); // Scale to 20 chars max
        
        std::cout << std::setw(5) << cycleNumbers[i] << " | " 
                 << std::setw(14) << std::fixed << std::setprecision(2) << hours << " | ";
        
        for (int j = 0; j < barLength; ++j) std::cout << "█";
        std::cout << " (" << std::setprecision(1) << hours << "h)\n";
    }
    
    std::cout << "\nPlots saved as 'simulation_results.png' and displayed using matplot++\n";
    std::cout << "Data also exported to 'simulation_data.csv' for external analysis\n";
}

void Simulation::createFirms() {
    // Create NUM_FIRMS firms
    for (int i = 0; i < NUM_FIRMS; ++i) {
        auto firm = std::make_shared<Firm>(nextFirmId++, firmNames[i], priceController);
        
        // Add some workers to each firm
        for (int j = 0; j < 3; ++j) {
            auto worker = std::make_shared<Worker>(j + 1, *firm);
            firm->addWorker(worker);
        }
        
        firms.push_back(firm);
    }
    
    firmSelector = std::uniform_int_distribution<>(0, firms.size() - 1);
    std::cout << "Created " << firms.size() << " firms with workers\n\n";
}

void Simulation::setProjectsPhase() {
    std::cout << "=== Phase 1: Companies Set Projects ===\n";
    
    std::uniform_int_distribution<> productSelector(0, products.size() - 1);
    std::uniform_int_distribution<> quantityDist(10, 100);
    
    // Each firm gets 1-2 projects this cycle
    for (auto& firm : firms) {
        int numProjects = (probability(gen) < 0.6) ? 1 : 2;
        
        for (int i = 0; i < numProjects; ++i) {
            std::string product = products[productSelector(gen)];
            int quantity = quantityDist(gen);
            
            Project project(product, quantity);
            firm->addProject(project);
            allProjects.push_back(project);
            
            std::cout << firm->getName() << " started project for " << product 
                     << ": " << quantity << " units (expected: " 
                     << firm->getProjectPrice(project) << " hours/unit)\n";
        }
    }
    std::cout << "\n";
}

void Simulation::innovationDiscoveryPhase() {
    std::cout << "=== Phase 2: Innovation Discovery ===\n";
    
    // Some firms discover innovations from others
    for (auto& firm : firms) {
        if (probability(gen) < innovation_discovery_rate) {
            // Pick a random firm that might have an innovation
            auto otherFirm = firms[firmSelector(gen)];
            if (otherFirm != firm) {
                // Pick a random product that we might discover innovations for
                std::uniform_int_distribution<> productSelector(0, products.size() - 1);
                std::string product = products[productSelector(gen)];
                
                // Check if the other firm has projects for this product and we do too
                if (otherFirm->hasProjectsForProduct(product) && firm->hasProjectsForProduct(product)) {
                    // Get most recent project costs for comparison
                    auto otherProjects = otherFirm->getProjectsForProduct(product);
                    auto& ourProjects = firm->getProjectsForProduct(product);
                    
                    if (!otherProjects.empty() && !ourProjects.empty()) {
                        const Project& otherProject = otherProjects.back();
                        Project& ourProject = ourProjects.back();
                        
                        if (otherProject.quantity > 0 && ourProject.quantity > 0) {
                            double otherCostPerUnit = otherProject.actualCost / otherProject.quantity;
                            double ourCostPerUnit = ourProject.actualCost / ourProject.quantity;
                            
                            if (otherCostPerUnit < ourCostPerUnit && otherProject.actualCost > 0) {
                                // Apply the innovation to our project
                                double newCost = otherCostPerUnit * ourProject.quantity;
                                ourProject.actualCost = newCost;
                                
                                std::cout << firm->getName() << " discovered " << otherFirm->getName() 
                                            << "'s innovation for " << product 
                                            << " (reduced from " << ourCostPerUnit << " to " << otherCostPerUnit << " hours/unit)\n";
                            }
                        }
                    }
                }
            }
        }
    }
    std::cout << "\n";
}

void Simulation::randomInnovationPhase() {
    std::cout << "=== Phase 3: Worker-Driven Innovations ===\n";
    
    // Worker-driven innovations based on research (15% per worker rate)
    for (auto& firm : firms) {
        if (probability(gen) < employee_suggestion_rate) {
            auto& projects = firm->getProjects();
            if (!projects.empty()) {
                // Pick a random project to innovate on
                std::uniform_int_distribution<> projectSelector(0, projects.size() - 1);
                auto& project = projects[projectSelector(gen)];
                firm->findInnovation(project);
            }
        }
    }
    std::cout << "\n";
}

void Simulation::productionPhase() {
    std::cout << "=== Phase 4: Production ===\n";
    
    // Simulate actual production - use the firm's assumed labor time with some variation
    std::uniform_real_distribution<> variationDist(0.8, 1.2); // ±20% variation
    
    for (auto& firm : firms) {
        // Work directly with the firm's project history to avoid copy issues
        auto& projectHistory = const_cast<std::unordered_map<std::string, std::vector<Project>>&>(firm->getProjectHistory());
        
        for (auto& [productName, projects] : projectHistory) {
            for (auto& project : projects) {
                if (project.actualCost == 0) { // Only work on new projects
                    double expectedHoursPerUnit = firm->getProjectPrice(project);
                    double actualHoursPerUnit = expectedHoursPerUnit * variationDist(gen);
                    double totalHours = actualHoursPerUnit * project.quantity;
                    
                    project.actualCost = totalHours;  // Set actualCost directly
                    
                    std::cout << firm->getName() << " produced " << project.quantity 
                             << " " << project.productName << " using " << totalHours 
                             << " hours (" << actualHoursPerUnit << " hours/unit)\n";
                }
            }
        }
    }
    std::cout << "\n";
}

void Simulation::priceControllerPhase() {
    std::cout << "=== Phase 5: Price Controller Update ===\n";
    
    // Collect all completed projects
    std::vector<Project> completedProjects;
    for (const auto& firm : firms) {
        const auto& projects = firm->getProjects();
        for (const auto& project : projects) {
            if (project.actualCost > 0) {
                completedProjects.push_back(project);
            }
        }
    }
    
    std::cout << "Analyzing " << completedProjects.size() << " completed projects...\n";
    
    // Update current costs based on actual production data
    priceController.updateCurrentCosts(completedProjects);
    
    // Show current vs official prices
    std::cout << "\nCurrent costs vs Official prices:\n";
    for (const auto& product : products) {
        double official = priceController.getOfficialPrice(product);
        double current = priceController.getCurrentCost(product);
        double savings = official - current;
        double percentage = (savings / official) * 100.0;
        
        std::cout << product << ": Official=" << official 
                 << ", Current=" << current 
                 << " (savings: " << savings << " hours, " << percentage << "%)\n";
    }
    
    // Update official prices if conditions are met
    std::cout << "\nChecking for price updates...\n";
    priceController.updateOfficialPrices(threshold_percentage_firms, threshold_percentage_products);
    std::cout << "\n";
}

void Simulation::runCycle(int cycleNumber) {
    std::cout << "==================== CYCLE " << cycleNumber << " ====================\n";
    
    // Phase 0: Check for economic shocks
    if (shouldTriggerShock(cycleNumber)) {
        triggerEconomicShock(cycleNumber);
    }
    cyclesSinceLastShock++;
    
    // Phase 1: Reset firm labor time tracking for this cycle
    for (auto& firm : firms) {
        firm->resetCycleLaborTime();
    }
    
    // Phase 2: Each firm creates projects
    std::cout << "=== Phase 2: Project Creation ===\n";
    for (auto& firm : firms) {
        firm->create_projects();
    }
    std::cout << "\n";
    
    // Phase 3: Each firm executes projects (includes learning, innovation, production)
    std::cout << "=== Phase 3: Project Execution ===\n";
    for (auto& firm : firms) {
        firm->execute_projects(firms);
    }
    std::cout << "\n";
    
    // Phase 4: Record labor time for this cycle
    std::cout << "=== Phase 4: Labor Time Recording ===\n";
    for (auto& firm : firms) {
        double cycleLaborTime = firm->getTotalLaborTimeSpent() - 
            (firm->getLaborTimeHistory().size() > 0 ? 
             std::accumulate(firm->getLaborTimeHistory().begin(), firm->getLaborTimeHistory().end(), 0.0) : 0.0);
        firm->recordLaborTimeForCycle(cycleLaborTime);
        
        std::cout << firm->getName() << " labor time this cycle: " << cycleLaborTime << " hours\n";
    }
    std::cout << "\n";
    
    // Phase 5: Price Controller updates
    priceControllerPhase();
    trackPricesAndWorkDay(cycleNumber);
    
    std::cout << "Cycle " << cycleNumber << " completed.\n\n";
    
    // Brief pause for readability
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void Simulation::showSummary() {
    std::cout << "=== SIMULATION SUMMARY ===\n";
    std::cout << "Total firms: " << firms.size() << "\n";
    std::cout << "Total projects created: " << nextProjectId - 1 << "\n";
    
    std::cout << "\nInitial official prices:\n";
    for (const auto& product : products) {
        std::cout << product << ": " << initialPrices[product] << " hours\n";
    }
    
    std::cout << "\nFinal official prices:\n";
    for (const auto& product : products) {
        double initial = initialPrices[product];
        double final = priceController.getOfficialPrice(product);
        double delta = final - initial;
        double percentChange = (initial > 0) ? (delta / initial) * 100.0 : 0.0;
        
        std::cout << product << ": " << final << " hours (delta: " << std::showpos 
                 << delta << std::noshowpos << " hours, " << std::showpos 
                 << percentChange << std::noshowpos << "%)\n";
    }
    
    std::cout << "\nFinal current costs (actual productivity):\n";
    for (const auto& product : products) {
        double initial = initialPrices[product];
        double current = priceController.getCurrentCost(product);
        double delta = current - initial;
        double percentChange = (initial > 0) ? (delta / initial) * 100.0 : 0.0;
        
        std::cout << product << ": " << current << " hours (delta: " << std::showpos 
                 << delta << std::noshowpos << " hours, " << std::showpos 
                 << percentChange << std::noshowpos << "%)\n";
    }
    
    std::cout << "\nFirm production summary:\n";
    for (const auto& firm : firms) {
        std::cout << firm->getName() << ": " << firm->getProjects().size() 
                 << " projects, " << firm->getTotalLaborTimeSpent() << " total hours\n";
        
        // Show labor time history
        const auto& history = firm->getLaborTimeHistory();
        std::cout << "  Labor time by cycle: ";
        for (size_t i = 0; i < history.size(); ++i) {
            std::cout << "C" << (i + 1) << ":" << std::fixed << std::setprecision(1) << history[i] << "h ";
        }
        std::cout << "\n";
        
        // Show labor time by product
        std::cout << "  Labor time by product: ";
        for (const auto& product : products) {
            double productTime = firm->getLaborTimeForProduct(product);
            if (productTime > 0) {
                std::cout << product << ":" << std::fixed << std::setprecision(1) << productTime << "h ";
            }
        }
        std::cout << "\n";
    }
    
    // Show economic shock history
    if (!economicShockHistory.empty()) {
        std::cout << "\nEconomic shock history:\n";
        for (const auto& [cycle, shockType] : economicShockHistory) {
            std::cout << "  Cycle " << cycle << ": " << shockType << "\n";
        }
    } else {
        std::cout << "\nNo economic shocks occurred during this simulation.\n";
    }
}

void Simulation::run(int numCycles) {
    std::cout << "Starting Economic Simulation with Labor Time Value\n";
    std::cout << "================================================\n\n";
    
    // Add cycle 0 to show the baseline of 8 hours before any simulation
    cycleNumbers.push_back(0);
    workDayHours.push_back(8.0);  // Baseline work day
    
    // Track initial prices for cycle 0
    for (const auto& product : products) {
        priceHistory[product].push_back(initialPrices[product]);
    }
    
    std::cout << "Cycle 0 (Baseline): Work day = 8.0 hours\n\n";
    
    for (int cycle = 1; cycle <= numCycles; ++cycle) {
        runCycle(cycle);
    }
    
    showSummary();
    generatePlots(); // Generate original plots
    
    // Generate new specialized plots
    generateProductWorkdayPlot("shirts");  // Example: shirts vs workday
    generateAveragePriceWorkdayPlot();     // Average price vs workday
    generateProductivityPieChart();        // Productivity by sector pie chart
    
    std::cout << "\nSimulation completed successfully!\n";
}

// Economic shock methods
bool Simulation::shouldTriggerShock(int cycleNumber) {
    // Don't trigger shocks in the first cycle or too frequently
    if (cycleNumber == 1 || cyclesSinceLastShock < 2) {
        return false;
    }
    
    // Increase probability slightly as cycles pass without shocks
    double adjustedProbability = shockProbabilityPerCycle + (cyclesSinceLastShock * 0.02);
    return probability(gen) < adjustedProbability;
}

void Simulation::triggerEconomicShock(int cycleNumber) {
    std::cout << "\n*** ECONOMIC SHOCK DETECTED ***\n";
    
    // Random shock type
    std::uniform_int_distribution<> shockTypeDist(0, 2);
    int shockType = shockTypeDist(gen);
    
    // Random severity (0.1 to 0.4 - 10% to 40% impact)
    std::uniform_real_distribution<> severityDist(0.1, 0.4);
    double severity = severityDist(gen);
    
    std::string shockName;
    switch (shockType) {
        case 0:
            shockName = "Productivity Crisis";
            applyProductivityShock(severity);
            break;
        case 1:
            shockName = "Demand Collapse";
            applyDemandShock(severity);
            break;
        case 2:
            shockName = "Supply Chain Disruption";
            applySupplyChainShock(severity);
            break;
    }
    
    economicShockHistory.push_back({cycleNumber, shockName});
    cyclesSinceLastShock = 0;
    
    std::cout << "Shock Type: " << shockName << " (Severity: " << (severity * 100.0) << "%)\n";
    std::cout << "*** END ECONOMIC SHOCK ***\n\n";
}

void Simulation::applyProductivityShock(double severity) {
    std::cout << "Applying productivity shock - widespread efficiency losses\n";
    
    // Temporarily increase official prices to simulate loss of productivity
    for (const auto& product : products) {
        double currentPrice = priceController.getOfficialPrice(product);
        double newPrice = currentPrice * (1.0 + severity);
        // Note: This modifies internal prices temporarily - 
        // in a real implementation you'd want a temporary modifier system
        std::cout << "  " << product << " labor time increased from " << currentPrice 
                  << " to " << newPrice << " hours due to productivity loss\n";
    }
    
    // Reduce innovation probability temporarily
    // This would need access to firm innovation rates in a full implementation
    std::cout << "  Innovation rates temporarily reduced by " << (severity * 100.0) << "%\n";
}

void Simulation::applyDemandShock(double severity) {
    std::cout << "Applying demand shock - reduced project creation\n";
    
    // Reduce number of projects firms will create this cycle
    // In a full implementation, this would modify firm behavior
    std::uniform_real_distribution<> reductionDist(0.0, 1.0);
    
    for (auto& firm : firms) {
        if (reductionDist(gen) < severity) {
            std::cout << "  " << firm->getName() << " reducing project creation due to low demand\n";
            // In practice, you'd modify the firm's project creation logic
        }
    }
    
    std::cout << "  " << (int)(severity * firms.size()) << " firms affected by demand reduction\n";
}

void Simulation::applySupplyChainShock(double severity) {
    std::cout << "Applying supply chain shock - increased production costs\n";
    
    // Increase actual production costs by adding random delays/inefficiencies
    std::uniform_real_distribution<> costIncreaseDist(1.0, 1.0 + severity);
    
    // This would ideally modify production costs in real-time
    // For now, we'll track it and apply it to current cycle
    std::cout << "  Production costs increased by " << (severity * 100.0) << "% on average\n";
    std::cout << "  Supply chain bottlenecks affecting " << (int)(severity * products.size()) 
              << " product categories\n";
    
    // In a full implementation, you'd modify the production phase to apply these costs
}

// Categorize products by labor intensity (healthcare = high, manufacturing = medium, etc.)
void Simulation::categorizeProductsByLaborIntensity() {
    // Define sectors and their labor intensity (1.0 = baseline, >1.0 = more labor intensive)
    sectorLaborIntensity["Manufacturing"] = 1.0;     // Baseline - shirts, shoes, shorts
    sectorLaborIntensity["Food Production"] = 1.2;   // Moderate labor intensity - apples, bread
    sectorLaborIntensity["Furniture/Construction"] = 0.8; // Lower labor intensity (more automated) - chairs, tables
    
    // Categorize products by sector
    productSectors["Manufacturing"] = {"shirts", "shoes", "shorts"};
    productSectors["Food Production"] = {"apples", "bread"};
    productSectors["Furniture/Construction"] = {"chairs", "tables"};
    
    // Initialize productivity gains tracking
    for (const auto& [sector, products] : productSectors) {
        sectorProductivityGains[sector] = 0.0;
    }
}

// Generate plot of specific product price vs workday
void Simulation::generateProductWorkdayPlot(const std::string& productName) {
    if (priceHistory.find(productName) == priceHistory.end()) {
        std::cout << "Product " << productName << " not found in price history.\n";
        return;
    }
    
    using namespace matplot;
    
    std::vector<double> cycles;
    for (int cycle : cycleNumbers) {
        cycles.push_back(static_cast<double>(cycle));
    }
    
    auto fig = figure(true);
    fig->size(1000, 600);
    
    // Plot product price
    auto p1 = plot(cycles, priceHistory[productName], "b-o");
    p1->line_width(2);
    p1->marker_size(6);
    xlabel("Cycle");
    ylabel(productName + " Price (Labor Hours)");
    title("Price vs Work Day: " + productName);
    
    // Add work day as second line (scaled for visibility)
    hold(on);
    auto p2 = plot(cycles, workDayHours, "r-s");
    p2->line_width(2);
    p2->marker_size(6);
    
    legend({productName + " Price", "Work Day Hours"});
    grid(on);
    save(productName + "_vs_workday.png");
    
    std::cout << "Generated plot: " << productName << " price vs work day\n";
}

// Generate plot of average price vs workday
void Simulation::generateAveragePriceWorkdayPlot() {
    using namespace matplot;
    
    // Calculate average price across all products for each cycle
    std::vector<double> averagePrices;
    for (size_t i = 0; i < cycleNumbers.size(); ++i) {
        double sum = 0.0;
        int count = 0;
        for (const auto& product : products) {
            if (i < priceHistory[product].size()) {
                sum += priceHistory[product][i];
                count++;
            }
        }
        averagePrices.push_back(count > 0 ? sum / count : 0.0);
    }
    
    std::vector<double> cycles;
    for (int cycle : cycleNumbers) {
        cycles.push_back(static_cast<double>(cycle));
    }
    
    auto fig = figure(true);
    fig->size(1000, 600);
    
    auto p1 = plot(cycles, averagePrices, "b-o");
    p1->line_width(3);
    p1->marker_size(8);
    xlabel("Cycle");
    ylabel("Average Price (Labor Hours)");
    title("Average Price vs Work Day");
    
    // Add work day as second line
    hold(on);
    auto p2 = plot(cycles, workDayHours, "r-s");
    p2->line_width(3);
    p2->marker_size(8);
    
    legend({"Average Price", "Work Day Hours"});
    grid(on);
    save("average_price_vs_workday.png");
    
    std::cout << "Generated plot: Average price vs work day\n";
}

// Generate pie chart showing productivity changes by sector
void Simulation::generateProductivityPieChart() {
    // Calculate productivity gains by sector
    for (const auto& [sector, productList] : productSectors) {
        double totalGain = 0.0;
        int productCount = 0;
        
        for (const auto& product : productList) {
            double initialPrice = initialPrices[product];
            double currentPrice = priceController.getCurrentCost(product);
            
            if (initialPrice > 0 && currentPrice > 0 && currentPrice < initialPrice) {
                double gain = (initialPrice - currentPrice) / initialPrice;
                // Adjust for labor intensity
                gain /= sectorLaborIntensity[sector];
                totalGain += gain;
                productCount++;
            }
        }
        
        sectorProductivityGains[sector] = productCount > 0 ? (totalGain / productCount) * 100.0 : 0.0;
    }
    
    using namespace matplot;
    
    std::vector<double> gains;
    std::vector<std::string> labels;
    
    for (const auto& [sector, gain] : sectorProductivityGains) {
        gains.push_back(gain);
        labels.push_back(sector + " (" + std::to_string(int(gain)) + "%)");
    }
    
    auto fig = figure(true);
    fig->size(800, 600);
    
    pie(gains, labels);
    title("Productivity Gains by Sector");
    save("productivity_pie_chart.png");
    
    std::cout << "Generated pie chart: Productivity gains by sector\n";
    
    // Console output
    std::cout << "\n=== PRODUCTIVITY GAINS BY SECTOR ===\n";
    for (const auto& [sector, gain] : sectorProductivityGains) {
        std::cout << sector << ": " << std::fixed << std::setprecision(1) << gain << "% improvement\n";
        std::cout << "  Labor Intensity Factor: " << sectorLaborIntensity[sector] << "\n";
        std::cout << "  Products: ";
        for (const auto& product : productSectors[sector]) {
            std::cout << product << " ";
        }
        std::cout << "\n\n";
    }
} 
