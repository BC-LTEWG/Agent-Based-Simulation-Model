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
    : id(id), name(name), laborTimeHoursSpent(0.0), priceController(priceController) {}

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
    std::uniform_real_distribution<double> distribution(0.1, 0.3);
    
    double reduction = distribution(generator);
    double oldCost = project.actualCost;
    project.actualCost = project.actualCost * (1.0 - reduction);
    
    std::cout << "Innovation at " << name << "! Reduced labor time for " 
              << project.productName << " from " << oldCost 
              << " to " << project.actualCost << " hours\n";
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












