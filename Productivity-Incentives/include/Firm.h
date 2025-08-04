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
    std::string productName;
    int quantity;
    double actualCost; // Total actual cost in labor hours
    
    Project(const std::string& name, int qty, double cost = 0.0)
        : productName(name), quantity(qty), actualCost(cost) {}
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
    
    // Main methods per specification
    void create_projects();
    void execute_projects(const std::vector<std::shared_ptr<Firm>>& allFirms);
    
    // Helper methods
    void findInnovation(Project& project); // Apply innovation to reduce labor time
    void updateProjectHours(const std::string& productName, double hours);
    
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
    std::vector<Project>& getProjectsForProduct(const std::string& productName);
    bool hasProjectsForProduct(const std::string& productName) const;
    
private:
    int id;
    std::string name;
    double laborTimeHoursSpent;
    std::vector<std::shared_ptr<Worker>> workers;
    std::unordered_map<std::string, std::vector<Project>> project_history;
    PriceController & priceController;

};
