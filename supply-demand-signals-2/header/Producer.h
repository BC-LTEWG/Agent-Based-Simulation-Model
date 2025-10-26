#pragma once
#include "Firm.h"
#include "Product.h"
#include "Person.h"
#include "Machine.h"
#include <string>
#include <vector>
#include <unordered_map>

class Producer : public Firm {
  private:
    std::vector<Product*> products;
    std::unordered_map<Product*, int> inventory;
    
  public:
    Producer() : Firm() {}
    Producer(std::vector<Machine*> machines, std::vector<Person*> workforce) 
        : Firm(machines, workforce, {}) {}
    Producer(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) 
        : Firm(machines, workforce, plans) {}


    
};
