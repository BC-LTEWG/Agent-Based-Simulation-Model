#include "../include/Project.h"
#include "../include/Worker.h"
#include <algorithm>
#include <iostream>

Project::Project(int id, int totalLaborTimeHoursForProject, int numWorkersNeeded)
    : id(id), totalLaborTimeHoursForProject(totalLaborTimeHoursForProject),
      numWorkersNeeded(numWorkersNeeded), completedHours(0) {
}

int Project::getId() const {
    return id;
}

int Project::getTotalLaborTimeHoursForProject() const {
    return totalLaborTimeHoursForProject;
}

int Project::getNumWorkersNeeded() const {
    return numWorkersNeeded;
}

int Project::getLaborTimeHoursPerWorker() const {
    if (numWorkersNeeded == 0) return 0;
    return totalLaborTimeHoursForProject / numWorkersNeeded;
}

void Project::addWorker(Worker* worker) {
    if(!worker) {
        std::cout << "Null ptr error\n";
    }

    if (worker && assignedWorkers.size() < static_cast<size_t>(numWorkersNeeded)) {
        auto it = std::find(assignedWorkers.begin(), assignedWorkers.end(), worker);
        if (it == assignedWorkers.end()) {
            assignedWorkers.push_back(worker);
            worker->setCurrentProject(*this);
        }
    }
}

void Project::removeWorker(Worker* worker) {
    if(!worker) {
        std::cout << "Null ptr error\n";
    }

    auto it = std::find(assignedWorkers.begin(), assignedWorkers.end(), worker);
    if (it != assignedWorkers.end()) {
        assignedWorkers.erase(it);
        worker->setCurrentProject(*this); // This might need to be set to nullptr
    }
}

const std::vector<Worker*>& Project::getAssignedWorkers() const {
    return assignedWorkers;
}

void Project::updateProgress(int additionalHours) {
    completedHours += additionalHours;
    if (completedHours > totalLaborTimeHoursForProject) {
        completedHours = totalLaborTimeHoursForProject;
    }
}

int Project::getCompletedHours() const {
    return completedHours;
}

bool Project::isCompleted() const {
    return completedHours >= totalLaborTimeHoursForProject;
}

double Project::getCompletionPercentage() const {
    if (totalLaborTimeHoursForProject == 0) return 0.0;
    return (static_cast<double>(completedHours) / totalLaborTimeHoursForProject) * 100.0;
} 