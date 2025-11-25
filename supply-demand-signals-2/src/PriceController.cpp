#include "PriceController.h"
#include "Firm.h"
#include "Product.h"
#include <iostream>

void PriceController::update_prices() {

}

void PriceController::get_price(Product * product) {
    if (product == nullptr) {
        std::cerr << "Error: Cannot get price for null product" << std::endl;
        return;
    }
    
    auto it = price_history.find(product);
    if (it != price_history.end()) {
        std::cout << "Price for " << product->product_name 
                  << ": " << it->second << std::endl;
    } else {
        std::cout << "Price for " << product->product_name 
                  << ": " << product->price_per_unit << " (default)" << std::endl;
    }
}

void PriceController::set_price(Product * product, double price) {
    if (product == nullptr) {
        std::cerr << "Error: Cannot set price for null product" << std::endl;
        return;
    }
    
    if (price < 0) {
        std::cerr << "Error: Price cannot be negative" << std::endl;
        return;
    }
    
    price_history[product] = price;
    product->price_per_unit = price;
}

void PriceController::get_all_prices() {
    std::cout << "=== All Prices ===" << std::endl;
    
    if (price_history.empty()) {
        std::cout << "No prices set in history" << std::endl;
        return;
    }
    
    for (const auto& pair : price_history) {
        Product * product = pair.first;
        double price = pair.second;
        std::cout << product->product_name << ": " << price << std::endl;
    }
}

void PriceController::set_all_prices(std::unordered_map<Product *, double> prices) {
    for (const auto& pair : prices) {
        Product * product = pair.first;
        double price = pair.second;
        
        if (product != nullptr && price >= 0) {
            price_history[product] = price;
            product->price_per_unit = price;
        }
    }
}

void PriceController::update_plan(Plan * plan) {
    if (plan == nullptr) {
        std::cerr << "Error: Cannot update null plan" << std::endl;
        return;
    }
    
    plan_history.push_back(plan);
}

void PriceController::get_plan_history() {
    std::cout << "=== Plan History ===" << std::endl;
    
    if (plan_history.empty()) {
        std::cout << "No plans in history" << std::endl;
        return;
    }
    
    for (size_t i = 0; i < plan_history.size(); ++i) {
        Plan * plan = plan_history[i];
        if (plan != nullptr && plan->product != nullptr) {
            std::cout << "Plan " << i + 1 << ": "
                      << "Product: " << plan->product->product_name
                      << ", Quantity: " << plan->total_quantity
                      << ", Labor Hours: " << plan->labor_hours
                      << ", Remaining Hours: " << plan->total_hours_remaining
                      << std::endl;
        }
    }
}

