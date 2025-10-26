#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include "Firm.h"
#include "Person.h"
#include "Producer.h"

class Distributor : public Firm {
    private:
        std::vector<Producer&> suppliers;
        std::unordered_map<Product, std::pair<Producer&, int>> inventory;
        
    public:
        Distributor() : Firm() {}
        Distributor(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) : Firm(machines, workforce, plans) {}
        double getOutputRatio(Product& product) {};
        double plannedSatisfactionPerPerson(Product& product, Person& person) {};





};
