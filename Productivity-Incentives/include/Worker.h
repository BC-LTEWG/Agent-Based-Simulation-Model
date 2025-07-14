#pragma once
#include "Project.h"
#include <vector>
#include <string>
#include "Firm.h"
#include "Distributor.h"

struct WorkRecord {
    int workerId;
    int laborTimeHours;
    int projectId;
    double laborTimeValue;
};

struct GoodsService {
    int id;
    std::string name;
    double laborTimeValue;
    int quantity;
};

class Worker {
    public:
        Worker(int id, int laborTimeHoursWorked, int laborTimeCertificateWorth);

        // Basic getters
        int getId() const;
        int getLaborTimeHoursWorked() const;
        int getLaborTimeCertificateWorth() const;
        Project * getCurrentProject() const;
        void setCurrentProject(Project &project);
        
        // Labor time exchange with firms
        void provideWork(Firm& firm, int laborTimeHours);
        void receiveWorkRecord(const WorkRecord& record);
        
        // Labor time exchange with distributors
        void consumeGoodsServices(Distributor& distributor, const std::vector<GoodsService>& goods);
        void receiveGoodsServices(const std::vector<GoodsService>& goods);
        int buyGoodsServices(Distributor& distributor, const std::vector<GoodsService>& goods);
        
        // Labor time accounting
        void updateLaborTimeBalance(double laborTimeValue);
        double getTotalLaborTimeValue() const { return totalLaborTimeValue; }
        
        // Getters for exchange history
        const std::vector<WorkRecord>& getWorkRecords() const { return workRecords; }
        const std::vector<GoodsService>& getReceivedGoods() const { return receivedGoods; }

    private:
        int id;
        int laborTimeHoursWorked;
        int laborTimeCertificateWorth;
        double totalLaborTimeValue;
        Project * currentProject;
        std::vector<WorkRecord> workRecords;
        std::vector<GoodsService> receivedGoods;
};