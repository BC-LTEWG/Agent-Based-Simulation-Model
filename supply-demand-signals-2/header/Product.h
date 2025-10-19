#pragma once
#include <vector>
#include <string>

struct Product {
    std::string productName;
    double pricePerUnit;
    int orderSize;
  
    Product(const std::string name, double price, int order) : productName(name), pricePerUnit(price), orderSize(order) {};
};
