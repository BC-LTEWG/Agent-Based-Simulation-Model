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
    std::unordered_map<Product, std::vector<Plan&>> planHistory;

  public:
    Firm() : machines({}), workers({}), plans({}) {}
    Firm(std::vector<Person&> workforce) : machines({}), workers(workforce), plans({}) {}



    
};
