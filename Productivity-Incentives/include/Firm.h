#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <map>

// Forward declaration
class Worker;


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



class Firm {
    public:
        Firm(int id, std::string name);
        
        // Setters
        void addWorker(std::shared_ptr<Worker> worker);
        void updatePriceOfProject(Project project);
        void addProject(Project project);

        void setProjectprice(Project project, double price);

        void findInnovation(Project project);


        // Getters
        int getId() const;
        void getProjectsForFirm(Firm firm);
        void getWorkersForFirm(Firm firm);
        double getTotalLaborTimeValueOfProject(Project project) const;

    private:
        std::string name;
        int id;
        double laborTimeHoursSpent;
        std::vector<std::shared_ptr<Worker>> workers;
        std::vector<Project> projects;
};
