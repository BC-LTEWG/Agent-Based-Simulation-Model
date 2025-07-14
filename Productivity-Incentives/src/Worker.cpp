#include "../include/Worker.h"
#include "../include/Firm.h"
#include "../include/Distributor.h"
#include <algorithm>

Worker::Worker(int id, int laborTimeHoursWorked, int laborTimeCertificateWorth) 
    : id(id), laborTimeHoursWorked(laborTimeHoursWorked), 
      laborTimeCertificateWorth(laborTimeCertificateWorth),
      totalLaborTimeValue(0.0), currentProject(nullptr) {
}

int Worker::getId() const {
    return id;
}

int Worker::getLaborTimeHoursWorked() const {
    return laborTimeHoursWorked;
}

int Worker::getLaborTimeCertificateWorth() const {
    return laborTimeCertificateWorth;
}

Project* Worker::getCurrentProject() const {
    return currentProject;
}

void Worker::setCurrentProject(Project& project) {
    currentProject = &project;
}

void Worker::provideWork(Firm& firm, int laborTimeHours) {
    if (laborTimeHours > 0) {
        laborTimeHoursWorked += laborTimeHours;
        firm.receiveProject(*this, laborTimeHours);
    }
}

void Worker::receiveWorkRecord(const WorkRecord& record) {
    workRecords.push_back(record);
    totalLaborTimeValue += record.laborTimeValue;
}

void Worker::consumeGoodsServices(Distributor& distributor, const std::vector<GoodsService>& goods) {
    if (!goods.empty()) {
        distributor.supplyGoodsServices(*this, goods);
    }
}

void Worker::receiveGoodsServices(const std::vector<GoodsService>& goods) {
    for (const auto& good : goods) {
        receivedGoods.push_back(good);
        totalLaborTimeValue -= good.laborTimeValue; // Spending labor time value
    }
}

void Worker::updateLaborTimeBalance(double laborTimeValue) {
    totalLaborTimeValue += laborTimeValue;
    if (currentProject) {
        currentProject->adjustTotalLaborTimeHours(laborTimeValue);
    }
}

const std::vector<WorkRecord>& Worker::getWorkRecords() const {
    return workRecords;
}

const std::vector<GoodsService>& Worker::getReceivedGoods() const {
    return receivedGoods;
} 