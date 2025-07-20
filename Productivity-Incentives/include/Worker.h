#pragma once
#include <string>

// Forward declaration
class Firm;
struct Project;

class Worker {
public:
    Worker(int id, Firm& firm);
    
    // Getters
    int getId() const;
    int getLaborTimeHoursSpent() const;
    Firm& getFirmWorkingFor() const;
    Project* getCurrentProject() const;
    double getLaborTimeValue() const;
    
    // Setters
    void setLaborTimeHoursSpent(int hours);
    void setCurrentProject(Project* project);
    void setLaborTimeValue(double value);
    
    // Work methods
    void workOnProject(int hours);
    
private:
    int id;
    int laborTimeHoursSpent;
    Firm& firmWorkingFor;
    Project* currentProject;
    double laborTimeValue;
}; 