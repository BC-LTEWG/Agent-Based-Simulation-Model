#pragma once
#include <vector>
#include <string>

struct Product {
  std::string productName;
  double pricePerUnit;
  int orderSize;
  int numUnits;

  Product(const std::string& name, double price, int order, int units) : productName(name), pricePerUnit(price), orderSize(order), numUnits(units) {};
};
