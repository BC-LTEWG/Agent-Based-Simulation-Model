#include "../include/Distributor.h"
#include "../include/Firm.h"
#include "../include/Worker.h"
#include <algorithm>

Distributor::Distributor(int id, double laborTimeRate)
    : id(id), laborTimeRate(laborTimeRate), totalLaborTimeValue(0.0) {
}

void Distributor::receiveGoodsServices(Firm& firm, const std::vector<GoodsService>& goods) {
    for (const auto& good : goods) {
        availableGoods.push_back(good);
        totalLaborTimeValue += good.laborTimeValue;
    }
}

void Distributor::supplyRawMaterials(Firm& firm, const std::vector<GoodsService>& materials) {
    double totalValue = calculateLaborTimeValue(materials);
    
    // Remove materials from available goods
    for (const auto& material : materials) {
        auto it = std::find_if(availableGoods.begin(), availableGoods.end(),
            [&material](const GoodsService& g) { return g.id == material.id; });
        if (it != availableGoods.end()) {
            availableGoods.erase(it);
        }
    }
    
    firm.receiveRawMaterials(materials, totalValue);
    totalLaborTimeValue -= totalValue;
}

void Distributor::supplyGoodsServices(Worker& worker, const std::vector<GoodsService>& goods) {
    double totalValue = calculateLaborTimeValue(goods);
    
    // Remove goods from available inventory
    for (const auto& good : goods) {
        auto it = std::find_if(availableGoods.begin(), availableGoods.end(),
            [&good](const GoodsService& g) { return g.id == good.id; });
        if (it != availableGoods.end()) {
            availableGoods.erase(it);
        }
    }
    
    // Record consumption and update worker
    ConsumptionRecord record = recordConsumption(worker, goods);
    worker.receiveGoodsServices(goods);
    
    totalLaborTimeValue -= totalValue;
}

ConsumptionRecord Distributor::recordConsumption(Worker& worker, const std::vector<GoodsService>& consumed) {
    ConsumptionRecord record;
    record.workerId = worker.getId();
    record.consumedGoods = consumed;
    record.totalLaborTimeValue = calculateLaborTimeValue(consumed);
    
    consumptionRecords.push_back(record);
    return record;
}

double Distributor::calculateLaborTimeValue(const std::vector<GoodsService>& goods) {
    double totalValue = 0.0;
    for (const auto& good : goods) {
        totalValue += good.laborTimeValue;
    }
    return totalValue;
}

void Distributor::updateLaborTimeBalance(double laborTimeValue) {
    totalLaborTimeValue += laborTimeValue;
}

int Distributor::getId() const {
    return id;
}

double Distributor::getTotalLaborTimeValue() const {
    return totalLaborTimeValue;
}

const std::vector<GoodsService>& Distributor::getAvailableGoods() const {
    return availableGoods;
} 