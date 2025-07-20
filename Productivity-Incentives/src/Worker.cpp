#include "../include/Worker.h"
#include "../include/Firm.h"

Worker::Worker(int id, Firm& firm) 
    : id(id), laborTimeHoursSpent(0), firmWorkingFor(firm), currentProject(nullptr), laborTimeValue(0.0) {
}

int Worker::getId() const {
    return id;
}

int Worker::getLaborTimeHoursSpent() const {
    return laborTimeHoursSpent;
}

Firm& Worker::getFirmWorkingFor() const {
    return firmWorkingFor;
}

Project* Worker::getCurrentProject() const {
    return currentProject;
}

double Worker::getLaborTimeValue() const {
    return laborTimeValue;
}

void Worker::setLaborTimeHoursSpent(int hours) {
    laborTimeHoursSpent = hours;
}

void Worker::setCurrentProject(Project* project) {
    currentProject = project;
}

void Worker::setLaborTimeValue(double value) {
    laborTimeValue = value;
}

void Worker::workOnProject(int hours) {
    if (currentProject != nullptr && hours > 0) {
        laborTimeHoursSpent += hours;
        // Record the work with the firm
        firmWorkingFor.recordLaborTime(id, currentProject->projectId, hours);
    }
} 