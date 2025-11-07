#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Firm.h"
#include "Machine.h"
#include "Person.h"
#include "Product.h"

class Producer : public Firm {
private:
    std::vector<Product*> products;
    std::unordered_map<Product*, int> inventory;
    
public:
    Producer();
    Producer(std::vector<Machine*> machines, std::vector<Person*> workforce);
    Producer(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans);
};
