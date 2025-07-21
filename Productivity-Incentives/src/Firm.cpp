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
    projects.push_back(project);
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
    // Random innovation reduces labor time by 10-30%
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.1, 0.3);
    
    double reduction = dis(gen);
    double currentPrice = projectPrices[project.projectId];
    double newPrice = currentPrice * (1.0 - reduction);
    
    projectPrices[project.projectId] = newPrice;
    
    std::cout << "Innovation at " << name << "! Reduced labor time for " 
              << project.productName << " from " << currentPrice 
              << " to " << newPrice << " hours\n";
}

void Firm::drawPriceFromOfficial(Project& project) {
    double officialPrice = priceController.getOfficialPrice(project.productName);
    projectPrices[project.projectId] = officialPrice;
}

void Firm::updateProjectHours(int projectId, double hours) {
    for (auto& project : projects) {
        if (project.projectId == projectId) {
            project.actualHoursSpent += hours;
            laborTimeHoursSpent += hours;
            break;
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
    return projects;
}

std::vector<Project>& Firm::getProjects() {
    return projects;
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










