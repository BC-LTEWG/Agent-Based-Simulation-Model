#include "../include/Simulation.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <fstream>
#include <iomanip>
#include <matplot/matplot.h>

Simulation::Simulation() : gen(std::random_device{}()), probability(0.0, 1.0), nextProjectId(1), nextFirmId(1),
    priceController(products, {5, 8, 3, 1, 2, 12, 20}) {
    saveInitialPrices();
    createFirms();
}

void Simulation::saveInitialPrices() {
    for (const auto& product : products) {
        initialPrices[product] = priceController.getOfficialPrice(product);
        priceHistory[product] = std::vector<double>();
    }
}

double Simulation::calculateAverageWorkDay() {
    // Calculate average work day based on productivity improvements
    // Base work day is 8 hours, reduced by average productivity gains
    double totalProductivityGain = 0.0;
    int productCount = 0;
    
    for (const auto& product : products) {
        double initial = initialPrices[product];
        double current = priceController.getCurrentCost(product);
        if (initial > 0) {
            double savings = (initial - current) / initial;
            totalProductivityGain += savings;
            productCount++;
        }
    }
    
    double avgProductivityGain = (productCount > 0) ? totalProductivityGain / productCount : 0.0;
    double baseWorkDay = 8.0; // hours
    return baseWorkDay * (1.0 - avgProductivityGain);
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
        auto p = plot(cycles, prices, "-o");
        p->display_name(product);
        p->line_width(2);
    }
    
    xlabel("Cycle");
    ylabel("Labor Hours per Unit");
    title("Commodity Prices Over Time");
    legend();
    grid(on);
    
    // Second subplot: Work day hours
    subplot(2, 1, 2);
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
            
            Project project(nextProjectId++, product, quantity);
            firm->addProject(project);
            allProjects.push_back(project);
            
            std::cout << firm->getName() << " started project " << project.projectId 
                     << ": " << quantity << " " << product << " (expected: " 
                     << firm->getProjectPrice(project) << " hours/unit)\n";
        }
    }
    std::cout << "\n";
}

void Simulation::innovationDiscoveryPhase() {
    std::cout << "=== Phase 2: Innovation Discovery ===\n";
    
    // Some firms discover innovations from others
    for (auto& firm : firms) {
        if (probability(gen) < discovery_probability) {
            // Pick a random firm that might have an innovation
            auto otherFirm = firms[firmSelector(gen)];
            if (otherFirm != firm) {
                // Pick a random product that we might discover innovations for
                std::uniform_int_distribution<> productSelector(0, products.size() - 1);
                std::string product = products[productSelector(gen)];
                
                // Check if the other firm has projects for this product and we do too
                if (otherFirm->hasProjectsForProduct(product) && firm->hasProjectsForProduct(product)) {
                    double otherPrice = otherFirm->getMostRecentProductPrice(product);
                    double ourPrice = firm->getMostRecentProductPrice(product);
                    
                    if (otherPrice < ourPrice) {
                        // Apply the innovation to our most recent project of this product type
                        firm->setMostRecentProductPrice(product, otherPrice);
                        
                        std::cout << firm->getName() << " discovered " << otherFirm->getName() 
                                    << "'s innovation for " << product 
                                    << " (reduced from " << ourPrice << " to " << otherPrice << " hours)\n";
                    }
                }
            }
        }
    }
    std::cout << "\n";
}

void Simulation::randomInnovationPhase() {
    std::cout << "=== Phase 3: Random Innovations ===\n";
    
    // Random firms get innovations
    for (auto& firm : firms) {
        if (probability(gen) < innovation_probability) {
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
        auto& projects = firm->getProjects();
        for (auto& project : projects) {
            if (project.actualHoursSpent == 0) { // Only work on new projects
                double expectedHoursPerUnit = firm->getProjectPrice(project);
                double actualHoursPerUnit = expectedHoursPerUnit * variationDist(gen);
                double totalHours = actualHoursPerUnit * project.productQuantity;
                
                firm->updateProjectHours(project.projectId, totalHours);
                
                std::cout << firm->getName() << " produced " << project.productQuantity 
                         << " " << project.productName << " using " << totalHours 
                         << " hours (" << actualHoursPerUnit << " hours/unit)\n";
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
            if (project.actualHoursSpent > 0) {
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
    
    setProjectsPhase();
    innovationDiscoveryPhase();
    randomInnovationPhase();
    productionPhase();
    priceControllerPhase();
    trackPricesAndWorkDay(cycleNumber); // Track prices and work day after each cycle
    
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
    }
}

void Simulation::run(int numCycles) {
    std::cout << "Starting Economic Simulation with Labor Time Value\n";
    std::cout << "================================================\n\n";
    
    for (int cycle = 1; cycle <= numCycles; ++cycle) {
        runCycle(cycle);
    }
    
    showSummary();
    generatePlots(); // Generate plots after the simulation run
    std::cout << "\nSimulation completed successfully!\n";
} 
