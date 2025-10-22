#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>
#include "Firm.h"
#include "Person.h"

class Distributor : public Firm {
    private:
        std::vector<Producer&> suppliers;
        std::unordered_map<Product, std::pair<Producer&, int>> inventory;
        
    public:
        Distributor() : Firm() {}
        Distributor(std::vector<Person&> workforce) : Firm(workforce) {}





};
