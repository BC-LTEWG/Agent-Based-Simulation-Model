#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include "Firm.h"
#include "Worker.h"

class Distributor : public Firm {
    public:
        std::vector<Firm&> firms;
        std::size_t firmCount;
        int rawMaterials;
        int laborHours;
        int machineHours;
        
        Distributor(std::vector<Worker&> workers, std::vector<Firm&> firms, std::size_t workerCount, std::size_t firmCount, int rawMaterials, int laborHours, int machineHours) : Firm(workerCount, {}, workers, {}), firms(firms), firmCount(firmCount), rawMaterials(rawMaterials), laborHours(laborHours), machineHours(machineHours) {}

        std::vector<Firm&> getFirms() {};
        std::size_t getWorkerCount() {};

        void distributeMaterialsToFirms(int rawMaterials, int machineHours, Firm& firm) {};

    private:
        void restockInventory() {};

};
