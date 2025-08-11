#include "../include/Firm.h"
#include "../include/Worker.h"
#include "../include/PriceController.h"
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <random>

// Firm implementation
Firm::Firm(int id, std::string name, PriceController& priceController) 
    : id(id), name(name), laborTimeHoursSpent(0.0), priceController(priceController), currentCycleLaborTime(0.0) {}

void Firm::addWorker(std::shared_ptr<Worker> worker) {
    workers.push_back(worker);
}

void Firm::addProject(const Project& project) {
    project_history[project.productName].push_back(project);
}

void Firm::setProjectPrice([[maybe_unused]] const Project& project, [[maybe_unused]] double price) {
    // Price is now managed by PriceController, not per-project
    // This method is kept for compatibility but doesn't do anything
}

void Firm::updatePriceOfProject([[maybe_unused]] const Project& project) {
    // Price updates are now handled by PriceController
    // This method is kept for compatibility but doesn't do anything
}

void Firm::reportProjectPriceToPriceController(const Project& project) {
    // Report completed project data to price controller for average calculation
    std::cout << "Firm " << name << " completed project for " << project.productName 
              << " with actual cost " << project.actualCost << " hours\n";
}

void Firm::findInnovation(Project& project) {
    static std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.15, 0.4);  // 15-40% cost reduction
    
    double reduction = distribution(generator);
    double expectedCost = priceController.getOfficialPrice(project.productName) * project.quantity;
    project.actualCost = expectedCost * (1.0 - reduction);
    
    std::cout << "Innovation at " << name << "! Developed efficient method for " 
              << project.productName << " (" << (reduction * 100.0) << "% cost reduction)\n";
}

void Firm::create_projects() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> productSelector(0, 6);  // 7 products: 0-6
    static std::uniform_int_distribution<> quantityDist(10, 100);
    static std::uniform_real_distribution<> probability(0.0, 1.0);
    
    static std::vector<std::string> products = {"shirts", "shoes", "shorts", "apples", "bread", "chairs", "tables"};
    
    // Each firm gets 1-2 projects this cycle
    int numProjects = (probability(gen) < 0.6) ? 1 : 2;
    
    for (int i = 0; i < numProjects; ++i) {
        std::string product = products[productSelector(gen)];
        int quantity = quantityDist(gen);
        
        Project project(product, quantity);
        addProject(project);
        
        double expectedPrice = priceController.getOfficialPrice(product);
        std::cout << name << " created project for " << product 
                 << ": " << quantity << " units (expected: " 
                 << expectedPrice << " hours/unit)\n";
    }
}

void Firm::execute_projects(const std::vector<std::shared_ptr<Firm>>& allFirms) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> probability(0.0, 1.0);
    static std::uniform_real_distribution<> variationDist(0.8, 1.2); // ±20% variation
    
    // Work on projects
    auto& projectHistory = project_history;
    
    for (auto& [productName, projects] : projectHistory) {
        for (auto& project : projects) {
            if (project.actualCost == 0) { // Only work on new projects
                
                // 1. Learn from other firms (discovery phase)
                if (probability(gen) < 0.6) { // 60% chance to discover
                    for (const auto& otherFirm : allFirms) {
                        if (otherFirm.get() != this && otherFirm->hasProjectsForProduct(productName)) {
                            auto otherProjects = otherFirm->getProjectsForProduct(productName);
                            if (!otherProjects.empty()) {
                                const Project& otherProject = otherProjects.back();
                                if (otherProject.quantity > 0 && project.quantity > 0) {
                                    double otherCostPerUnit = otherProject.actualCost / otherProject.quantity;
                                    double ourPlannedCostPerUnit = priceController.getOfficialPrice(productName);
                                    
                                    if (otherCostPerUnit < ourPlannedCostPerUnit && otherProject.actualCost > 0) {
                                        double newTotalCost = otherCostPerUnit * project.quantity;
                                        project.actualCost = newTotalCost;
                                        
                                        std::cout << name << " learned from " << otherFirm->getName() 
                                                  << "'s innovation for " << productName 
                                                  << " (cost: " << otherCostPerUnit << " hours/unit)\n";
                                        continue; // Skip to next project
                                    }
                                }
                            }
                        }
                    }
                }
                
                // 2. Random innovation (if didn't learn from others)
                if (project.actualCost == 0 && probability(gen) < 0.4) { // 40% chance
                    findInnovation(project); // This sets actualCost with innovation
                }
                
                // 3. Normal production (if no learning or innovation)
                if (project.actualCost == 0) {
                    double expectedHoursPerUnit = priceController.getOfficialPrice(productName);
                    double actualHoursPerUnit = expectedHoursPerUnit * variationDist(gen);
                    project.actualCost = actualHoursPerUnit * project.quantity;
                    
                    std::cout << name << " produced " << project.quantity 
                             << " " << project.productName << " using " << project.actualCost 
                             << " hours (" << actualHoursPerUnit << " hours/unit)\n";
                }
                
                laborTimeHoursSpent += project.actualCost;
                currentCycleLaborTime += project.actualCost;
                
                // Track labor time by product
                laborTimeByProduct[project.productName] += project.actualCost;
            }
        }
    }
}

void Firm::updateProjectHours(const std::string& productName, double hours) {
    // Update the most recent project for this product
    auto it = project_history.find(productName);
    if (it != project_history.end() && !it->second.empty()) {
        it->second.back().actualCost += hours;
        laborTimeHoursSpent += hours;
    }
}

int Firm::getId() const {
    return id;
}

std::string Firm::getName() const {
    return name;
}

double Firm::getProjectPrice(const Project& project) const {
    // Return official price from PriceController - firms plan based on official targets
    return priceController.getOfficialPrice(project.productName);
}

const std::vector<Project>& Firm::getProjects() const {
    // For compatibility, return all projects as a single vector
    static std::vector<Project> allProjects;
    allProjects.clear();
    
    for (const auto& [productName, projects] : project_history) {
        allProjects.insert(allProjects.end(), projects.begin(), projects.end());
    }
    
    return allProjects;
}

std::vector<Project>& Firm::getProjects() {
    // For compatibility, return all projects as a single vector
    static std::vector<Project> allProjects;
    allProjects.clear();
    
    for (const auto& [productName, projects] : project_history) {
        allProjects.insert(allProjects.end(), projects.begin(), projects.end());
    }
    
    return allProjects;
}

const std::vector<std::shared_ptr<Worker>>& Firm::getWorkers() const {
    return workers;
}

double Firm::getTotalLaborTimeValueOfProject(const Project& project) const {
    double pricePerUnit = getProjectPrice(project);
    return project.quantity * pricePerUnit;
}

double Firm::getTotalLaborTimeSpent() const {
    return laborTimeHoursSpent;
}

// New methods for project history
const std::unordered_map<std::string, std::vector<Project>>& Firm::getProjectHistory() const {
    return project_history;
}

std::vector<Project> Firm::getProjectsForProduct(const std::string& productName) const {
    auto it = project_history.find(productName);
    return (it != project_history.end()) ? it->second : std::vector<Project>();
}

std::vector<Project>& Firm::getProjectsForProduct(const std::string& productName) {
    return project_history[productName];
}



bool Firm::hasProjectsForProduct(const std::string& productName) const {
    auto it = project_history.find(productName);
    return it != project_history.end() && !it->second.empty();
}

// Enhanced labor time tracking methods
double Firm::getLaborTimeForCycle(int cycle) const {
    if (cycle >= 0 && cycle < static_cast<int>(laborTimeHistory.size())) {
        return laborTimeHistory[cycle];
    }
    return 0.0;
}

double Firm::getLaborTimeForProduct(const std::string& productName) const {
    auto it = laborTimeByProduct.find(productName);
    return (it != laborTimeByProduct.end()) ? it->second : 0.0;
}

const std::vector<double>& Firm::getLaborTimeHistory() const {
    return laborTimeHistory;
}

void Firm::recordLaborTimeForCycle(double hours) {
    laborTimeHistory.push_back(hours);
    currentCycleLaborTime = 0.0; // Reset for next cycle
}

void Firm::resetCycleLaborTime() {
    currentCycleLaborTime = 0.0;
}












