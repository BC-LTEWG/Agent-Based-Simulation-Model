#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include "PriceController.h"
#include "Worker.h"
// Forward declaration to avoid circular dependency

struct Project {
    int projectId;
    std::string productName;
    int productQuantity;
    std::string unit;
    double account; // Project account
    double actualHoursSpent; // Track actual hours spent
    
    Project(int id, const std::string& name, int quantity, const std::string& unitType = "units")
        : projectId(id), productName(name), productQuantity(quantity), unit(unitType), 
          account(0.0), actualHoursSpent(0.0) {}
          
    
};

class Firm {
public:
    Firm(int id, std::string name, PriceController& priceController);
    
    // Setters
    void addWorker(std::shared_ptr<Worker> worker);
    void updatePriceOfProject(const Project& project);
    void addProject(const Project& project);
    void setProjectPrice(const Project& project, double price);
    void reportProjectPriceToPriceController(const Project& project);
    
    void findInnovation(Project& project); // Apply innovation to reduce labor time
    void updateProjectHours(int projectId, double hours);
    
    // Getters
    int getId() const;
    std::string getName() const;
    double getProjectPrice(const Project& project) const;
    const std::vector<Project>& getProjects() const;
    std::vector<Project>& getProjects();
    const std::vector<std::shared_ptr<Worker>>& getWorkers() const;
    double getTotalLaborTimeValueOfProject(const Project& project) const;
    double getTotalLaborTimeSpent() const;
    
    // New methods for project history
    const std::unordered_map<std::string, std::vector<Project>>& getProjectHistory() const;
    std::vector<Project> getProjectsForProduct(const std::string& productName) const;
    double getMostRecentProductPrice(const std::string& productName) const;
    bool hasProjectsForProduct(const std::string& productName) const;
    void setMostRecentProductPrice(const std::string& productName, double price);
    
private:
    int id;
    std::string name;
    double laborTimeHoursSpent;
    std::vector<std::shared_ptr<Worker>> workers;
    std::unordered_map<std::string, std::vector<Project>> project_history;
    PriceController & priceController;
    std::map<int, double> projectPrices;
};
