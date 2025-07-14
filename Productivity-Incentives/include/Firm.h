#pragma once
#include <vector>
#include <memory>

class Worker;
class Project;
class Distributor;

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

class Firm {
    public:
        Firm(int id, double laborTimeRate);
        
        // Labor time exchange with workers
        void receiveWork(Worker& worker, int laborTimeHours);
        WorkRecord issueWorkRecord(Worker& worker, int laborTimeHours);
        
        // Labor time exchange with distributors
        void produceGoodsServices(std::vector<GoodsService>& goods);
        double calculateLaborTimeValue(const std::vector<GoodsService>& goods);
        
        // Raw materials from distributors
        void receiveRawMaterials(const std::vector<GoodsService>& materials, double laborTimeValue);
        
        // Getters
        int getId() const { return id; }
        double getTotalLaborTimeReceived() const { return totalLaborTimeReceived; }
        double getTotalLaborTimeValue() const { return totalLaborTimeValue; }
        
    private:
        int id;
        double laborTimeRate;
        double totalLaborTimeReceived;
        double totalLaborTimeValue;
        std::vector<WorkRecord> workRecords;
        std::vector<GoodsService> producedGoods;
        std::vector<GoodsService> rawMaterials;
};
