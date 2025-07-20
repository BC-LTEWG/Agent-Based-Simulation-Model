#include "../include/Firm.h"
#include <algorithm>

struct WorkRecord {
    int workerId;
    int laborTimeHours;
    int projectId;
    double laborTimeValue;
};

struct GoodsService {
    std::string name;
    int quantity;
    std::string unit;
    double laborTimeValue;
};

Firm::Firm(int id, Project project)
    : id(id), laborTimeHoursSpent(0.0) {
    projects.push_back(project);
}

void Firm::receiveWork(Worker& worker, int laborTimeHours) {
    if (laborTimeHours > 0) {
        totalLaborTimeReceived += laborTimeHours;
        
        // Create and issue work record
        WorkRecord record = issueWorkRecord(worker, laborTimeHours);
        worker.receiveWorkRecord(record);
    }
}

WorkRecord Firm::issueWorkRecord(Worker& worker, int laborTimeHours) {
    WorkRecord record;
    record.workerId = worker.getId();
    record.laborTimeHours = laborTimeHours;
    record.projectId = (worker.getCurrentProject() ? worker.getCurrentProject()->getId() : 0);
    record.laborTimeValue = laborTimeHours * laborTimeRate;
    
    workRecords.push_back(record);
    totalLaborTimeValue += record.laborTimeValue;
    
    return record;
}

void Firm::produceGoodsServices(std::vector<GoodsService>& goods) {
    for (auto& good : goods) {
        // Calculate labor time value based on production requirements
        good.laborTimeValue = good.quantity * laborTimeRate;
        producedGoods.push_back(good);
    }
}

double Firm::calculateLaborTimeValue(const std::vector<GoodsService>& goods) {
    double totalValue = 0.0;
    for (const auto& good : goods) {
        totalValue += good.laborTimeValue;
    }
    return totalValue;
}

void Firm::receiveRawMaterials(const std::vector<GoodsService>& materials, double laborTimeValue) {
    for (const auto& material : materials) {
        rawMaterials.push_back(material);
    }
    
    // Update labor time balance (spending labor time value for raw materials)
    totalLaborTimeValue -= laborTimeValue;
}

int Firm::getId() const {
    return id;
}

double Firm::getTotalLaborTimeReceived() const {
    return totalLaborTimeReceived;
}

double Firm::getTotalLaborTimeValue() const {
    return totalLaborTimeValue;
} 