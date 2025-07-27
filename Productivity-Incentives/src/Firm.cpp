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
    // Set initial price from official prices
    projectPrices[project.projectId] = priceController.getOfficialPrice(project.productName);
}

void Firm::setProjectPrice(const Project& project, double price) {
    projectPrices[project.projectId] = price;
}

void Firm::updatePriceOfProject(const Project& project) {
    double officialPrice = priceController.getOfficialPrice(project.productName);
    projectPrices[project.projectId] = officialPrice;
}

void Firm::reportProjectPriceToPriceController(const Project& project) {
    // Report completed project data to price controller for average calculation
    std::cout << "Firm " << name << " completed project " << project.projectId 
              << " for " << project.productName << " using " << project.actualHoursSpent << " hours\n";
}

void Firm::findInnovation(Project& project) {
    static std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.1, 0.3);
    
    double reduction = distribution(generator);
    double currentPrice = projectPrices[project.projectId];
    double newPrice = currentPrice * (1.0 - reduction);
    
    projectPrices[project.projectId] = newPrice;
    
    std::cout << "Innovation at " << name << "! Reduced labor time for " 
              << project.productName << " from " << currentPrice 
              << " to " << newPrice << " hours\n";
}

void Firm::updateProjectHours(int projectId, double hours) {
    // Search through all product histories to find the project
    for (auto& [productName, projects] : project_history) {
        for (auto& project : projects) {
            if (project.projectId == projectId) {
                project.actualHoursSpent += hours;
                laborTimeHoursSpent += hours;
                return;
            }
        }
    }
}

int Firm::getId() const {
    return id;
}

std::string Firm::getName() const {
    return name;
}

double Firm::getProjectPrice(const Project& project) const {
    auto it = projectPrices.find(project.projectId);
    return (it != projectPrices.end()) ? it->second : 0.0;
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
    return project.productQuantity * pricePerUnit;
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

double Firm::getMostRecentProductPrice(const std::string& productName) const {
    auto it = project_history.find(productName);
    if (it != project_history.end() && !it->second.empty()) {
        const Project& mostRecentProject = it->second.back();
        return getProjectPrice(mostRecentProject);
    }
    return priceController.getOfficialPrice(productName);
}

bool Firm::hasProjectsForProduct(const std::string& productName) const {
    auto it = project_history.find(productName);
    return it != project_history.end() && !it->second.empty();
}

void Firm::setMostRecentProductPrice(const std::string& productName, double price) {
    auto it = project_history.find(productName);
    if (it != project_history.end() && !it->second.empty()) {
        const Project& mostRecentProject = it->second.back();
        projectPrices[mostRecentProject.projectId] = price;
    }
}










