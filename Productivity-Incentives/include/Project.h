#pragma once
#include <vector>

class Worker;

class Project {
    public:
        Project(int id, int totalLaborTimeHoursForProject, int numWorkersNeeded);
        
        // Getters
        int getId() const { return id; }
        int getTotalLaborTimeHoursForProject() const { return totalLaborTimeHoursForProject; }
        int getNumWorkersNeeded() const { return numWorkersNeeded; }
        int getLaborTimeHoursPerWorker() const { return totalLaborTimeHoursForProject / numWorkersNeeded; }
        
        // Worker management
        void addWorker(Worker* worker);
        void removeWorker(Worker* worker);
        const std::vector<Worker*>& getAssignedWorkers() const { return assignedWorkers; }
        
        // Progress tracking
        void updateProgress(int additionalHours);
        int getCompletedHours() const { return completedHours; }
        bool isCompleted() const { return completedHours >= totalLaborTimeHoursForProject; }
        double getCompletionPercentage() const { return (double)completedHours / totalLaborTimeHoursForProject * 100; }
        
        // Labor time adjustment
        void adjustTotalLaborTimeHours(double adjustment) { totalLaborTimeHoursForProject -= adjustment; }

    private:
        int id;
        int totalLaborTimeHoursForProject;
        int numWorkersNeeded;
        int completedHours;
        std::vector<Worker*> assignedWorkers;
};