#pragma once
#include <vector>
#include <memory>

class Firm;
class Worker;

struct GoodsService {
    int id;
    std::string name;
    double laborTimeValue;
    int quantity;
};

struct ConsumptionRecord {
    int workerId;
    std::vector<GoodsService> consumedGoods;
    double totalLaborTimeValue;
};

class Distributor {
    public:
        Distributor(int id, double laborTimeRate);

        // Labor time exchange with firms
        void receiveGoodsServices(Firm& firm, const std::vector<GoodsService>& goods);
        void supplyRawMaterials(Firm& firm, const std::vector<GoodsService>& materials);
        
        // Labor time exchange with workers
        void supplyGoodsServices(Worker& worker, const std::vector<GoodsService>& goods);
        ConsumptionRecord recordConsumption(Worker& worker, const std::vector<GoodsService>& consumed);
        
        // Labor time accounting
        double calculateLaborTimeValue(const std::vector<GoodsService>& goods);
        void updateLaborTimeBalance(double laborTimeValue);
        
        // Getters
        int getId() const { return id; }
        double getTotalLaborTimeValue() const { return totalLaborTimeValue; }
        const std::vector<GoodsService>& getAvailableGoods() const { return availableGoods; }

    private:
        int id;
        double laborTimeRate;
        double totalLaborTimeValue;
        std::vector<GoodsService> availableGoods;
        std::vector<GoodsService> rawMaterials;
        std::vector<ConsumptionRecord> consumptionRecords;
};
