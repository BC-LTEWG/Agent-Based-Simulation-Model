#pragma once
#include <vector>
#include <string>
#include <random>
#include "Society.h"
#include "Distributor.h"
#include "Firm.h"
#include "Worker.h"
#include "Machine.h"
#include "Product.h"

class Sim {
  public:
    Society society;
    Distributor distributor;
    std::vector<Product> products;
    int timeStep;
    int maxTimeSteps;
    std::mt19937 rng;
    
    Sim(Society society, Distributor distributor, std::vector<Product> products, int maxTimeSteps) 
      : society(society), distributor(distributor), products(products), timeStep(0), maxTimeSteps(maxTimeSteps), rng(std::random_device{}()) {}
    
    void runSimulation();
    void stepSimulation();
    bool isSimulationComplete() {return timeStep >= maxTimeSteps;};
    void updateProductivity();
    void updateSupplyDemand();
    void generateReports();
    
    int getCurrentTimeStep() {return timeStep;};
    int getMaxTimeSteps() {return maxTimeSteps;};
    Society& getSociety() {return society;};
    Distributor& getDistributor() {return distributor;};
};
