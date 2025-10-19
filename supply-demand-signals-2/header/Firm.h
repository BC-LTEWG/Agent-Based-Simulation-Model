#pragma once  
#include <string>
#include <vector>
#include "Person.h"
#include "Machine.h"
#include "Product.h"
#include <unordered_map>
#include <tuple>

class Firm;

struct Plan {
  Product& product;
  std::vector<Machine> machines;
  Firm& firm;
  int laborHours;
  int rawMaterials;
  int totalQuantity;
  int prd;
  int laborHoursRemaining;
  int rawMaterialsRemaining;
};

class Firm {
  public:
    std::vector<Machine*> machines;
    std::vector<Person*> workers;
    std::vector<Plan*> plans;
    std::unordered_map<Product*, std::vector<Plan*>> planHistory;

    Firm() {}
    Firm(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) : machines(machines), workers(workforce), plans(plans) {}
};
