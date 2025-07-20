#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <map>

class Worker;
struct GoodsService;
struct WorkRecord;

struct Project {
    std::string productName;
    int productQuantity;
    int projectId;
    std::string unit;
    std::map<Firm, double> priceOfProjectPerFirm;
};

struct PriceController {
    std::unordered_map<Project, double> avgPriceOfProject;

    void updateAvgPriceOfProject(Project project);
    double getAvgPriceOfProject(Project project);
};

struct Worker {
    int id;
    int laborTimeHoursSpent;
    Firm & firmWorkingFor;
    Project projectWorkingOn;
    double laborTimeValue;

    void setProjectForWorker(Project project);

};

class Firm {
    public:
        Firm(int id, Project project);
        
        // Setters
        void addWorker(Worker worker);
        void updatePriceOfProject(Project project);
        void addProject(Project project);

        // Getters
        int getId() const;
        double getTotalLaborTimeSpent() const;
        double getTotalLaborTimeValue() const;

    private:
        int id;
        double laborTimeHoursSpent;
        std::vector<Worker> workers;
        std::vector<Project> projects;
};
