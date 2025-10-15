#pragma once  
#include <string>
#include <vector>
#include "Person.h"
#include "Machine.h"
#include "Product.h"
#include <unordered_map>
#include <tuple>

// Forward declaration for Producer (alias for Firm)
class Firm;
using Producer = Firm;

struct Plan {
  Product& product;
  std::vector<Machine> machines;
  int laborHours;
  int rawMaterials;
  int totalQuantity;
  int prd;
};

class Firm {
  private:
    std::vector<Machine*> machines;
    std::vector<Person*> workers;
    std::vector<Plan*> plans;
    std::unordered_map<const Product&, std::vector<Plan&>> planHistory;

  public:
    Firm() {}
    Firm(std::vector<Machine*> machines, std::vector<Person*> workforce, std::vector<Plan*> plans) : machines(machines), workers(workforce), plans(plans) {}
};
