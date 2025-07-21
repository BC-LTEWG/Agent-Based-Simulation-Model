#include "../include/Firm.h"
#include "../include/Worker.h"
#include "../include/PriceController.h"
#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <chrono>
#include <thread>

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
    Simulation() : gen(std::random_device{}()), probability(0.0, 1.0), nextProjectId(1), nextFirmId(1) {
        priceController.initializeOfficialPrices();
        createFirms();
    }
    
    void createFirms() {
        // Create 10 firms
        for (int i = 0; i < 10; ++i) {
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
    
    void setProjectsPhase() {
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
    
    void innovationDiscoveryPhase() {
        std::cout << "=== Phase 2: Innovation Discovery ===\n";
        
        // Some firms discover innovations from others
        for (auto& firm : firms) {
            if (probability(gen) < DISCOVERY_PROBABILITY) {
                // Pick a random firm that might have an innovation
                auto otherFirm = firms[firmSelector(gen)];
                if (otherFirm != firm && !otherFirm->getProjects().empty()) {
                    // Find a project from the other firm to copy innovation from
                    const auto& otherProjects = otherFirm->getProjects();
                    if (!otherProjects.empty()) {
                        const auto& innovativeProject = otherProjects.back();
                        
                        // Find our project of the same product type
                        auto& ourProjects = firm->getProjects();
                        for (auto& project : ourProjects) {
                            if (project.productName == innovativeProject.productName) {
                                double otherPrice = otherFirm->getProjectPrice(innovativeProject);
                                double ourPrice = firm->getProjectPrice(project);
                                
                                if (otherPrice < ourPrice) {
                                    firm->setProjectPrice(project, otherPrice);
                                    std::cout << firm->getName() << " discovered " << otherFirm->getName() 
                                             << "'s innovation for " << project.productName 
                                             << " (reduced from " << ourPrice << " to " << otherPrice << " hours)\n";
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        std::cout << "\n";
    }
    
    void randomInnovationPhase() {
        std::cout << "=== Phase 3: Random Innovations ===\n";
        
        // Random firms get innovations
        for (auto& firm : firms) {
            if (probability(gen) < INNOVATION_PROBABILITY) {
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
    
    void productionPhase() {
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
    
    void priceControllerPhase() {
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
        priceController.updateOfficialPrices(THRESHOLD_PERCENTAGE, DELTA_THRESHOLD);
        std::cout << "\n";
    }
    
    void runCycle(int cycleNumber) {
        std::cout << "==================== CYCLE " << cycleNumber << " ====================\n";
        
        setProjectsPhase();
        innovationDiscoveryPhase();
        randomInnovationPhase();
        productionPhase();
        priceControllerPhase();
        
        std::cout << "Cycle " << cycleNumber << " completed.\n\n";
        
        // Brief pause for readability
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    void showSummary() {
        std::cout << "=== SIMULATION SUMMARY ===\n";
        std::cout << "Total firms: " << firms.size() << "\n";
        std::cout << "Total projects created: " << nextProjectId - 1 << "\n";
        
        std::cout << "\nFinal official prices:\n";
        for (const auto& product : products) {
            std::cout << product << ": " << priceController.getOfficialPrice(product) << " hours\n";
        }
        
        std::cout << "\nFirm production summary:\n";
        for (const auto& firm : firms) {
            std::cout << firm->getName() << ": " << firm->getProjects().size() 
                     << " projects, " << firm->getTotalLaborTimeSpent() << " total hours\n";
        }
    }
};

int main() {
    std::cout << "Starting Economic Simulation with Labor Time Value\n";
    std::cout << "================================================\n\n";
    
    Simulation sim;
    
    // Run simulation for several cycles
    const int NUM_CYCLES = 5;
    
    for (int cycle = 1; cycle <= NUM_CYCLES; ++cycle) {
        sim.runCycle(cycle);
    }
    
    sim.showSummary();
    
    std::cout << "\nSimulation completed successfully!\n";
    return 0;
}
