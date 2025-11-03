#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "Firm.h"
#include "Person.h"
#include "Producer.h"

class Distributor : public Firm {
private:
    std::vector<Producer&> suppliers;
    std::unordered_map<Product, std::pair<Producer&, int>> inventory;
    
public:
    Distributor();
    Distributor(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans);
    double get_output_ratio(Product& product);
    double planned_satisfaction_per_person(Product& product, Person& person);
};
