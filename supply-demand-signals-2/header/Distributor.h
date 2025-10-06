#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include "Firm.h"
#include "Worker.h"

class Distributor {
    public:
        std::vector<Worker&> workers;
        std::vector<Firm&> firms;
        std::size_t workerCount;
        std::size_t firmCount; // number of firms to distribute to
        int rawMaterials;
        int laborHours;
        int machineHours;
        std::unordered_map<std::string, int> finishedGoods;
        std::unordered_map<std::string, std::tuple<std::vector<Worker&>, std::vector<int>>> personNeeds;

        Distributor(std::vector<Worker&> workers, std::vector<Firm&> firms, std::size_t workerCount, std::size_t firmCount, int rawMaterials, int laborHours, int machineHours) : workers(workers), firms(firms), workerCount(workerCount), firmCount(firmCount), rawMaterials(rawMaterials), laborHours(laborHours), machineHours(machineHours) {}

        std::vector<Firm&> getFirms() {};
        std::vector<Worker&> getWorkerNeeds() {};
        std::size_t getWorkerCount() {};
        std::size_t getFirmCount() {};

        void distributeMaterialsToFirms(int rawMaterials, int machineHours, Firm& firm) {};
        void restockInventory() {};

};
