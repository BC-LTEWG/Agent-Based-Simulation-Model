#pragma once
#include <vector>
#include <string>

class Product {
  public:
    std::string productName;
    double pricePerUnit;
    int orderSize;
  
    Product(const std::string& name, double price, int order) : productName(name), pricePerUnit(price), orderSize(order), laborHoursRemaining(0), rawMaterialsRemaining(0) {};

  private:
    int laborHoursRemaining;
    int rawMaterialsRemaining;
  
  friend class Firm;
};
