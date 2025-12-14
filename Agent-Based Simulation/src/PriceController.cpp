#include <iostream>

#include "Distributor.h"
#include "Firm.h"
#include "PriceController.h"
#include "Product.h"
#include "Machine.h"
#include "Society.h"
#include "Agent.h"

void PriceController::on_time_step() {
    poll_prices();
}

void PriceController::update_prices() {

}

double PriceController::get_price(Product * product) {
    if (product == nullptr) {
        std::cerr << "Error: Cannot get price for null product" << std::endl;
        return 0.0;
    }
    
    auto it = price_history.find(product);
    if (it != price_history.end()) {
        std::cout << "Price for " << product->product_name 
                  << ": " << it->second << std::endl;
    } else {
        std::cout << "Price for " << product->product_name 
                  << ": " << product->price_per_unit << " (default)" << std::endl;
    }
	return (it != price_history.end()) ? it->second : product->price_per_unit;
}

double PriceController::get_price(Product * product, Firm * firm) {
    if (product == nullptr) {
        std::cerr << "Error: Cannot get price for null product" << std::endl;
        return 0.0;
    }

    if (firm != nullptr) {
        auto firm_it = firm_price_history.find(firm);
        if (firm_it != firm_price_history.end()) {
            auto it = firm_it->second.find(product);
            if (it != firm_it->second.end()) {
                return it->second;
            }
        }
    }

    return get_price(product);
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

void PriceController::set_price(Product * product, Firm * firm, double price) {
    if (product == nullptr) {
        std::cerr << "Error: Cannot set price for null product" << std::endl;
        return;
    }

    if (price < 0) {
        std::cerr << "Error: Price cannot be negative" << std::endl;
        return;
    }

    if (firm == nullptr) {
        set_price(product, price);
        return;
    }

    firm_price_history[firm][product] = price;
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
        if (plan != nullptr && plan->order->product != nullptr) {
            std::cout << "Plan " << i + 1 << ": "
                      << "Product: " << plan->order->product->product_name
                      << ", Quantity: " << plan->order->quantity
                      << ", Labor Hours: " << plan->labor_hours
                      << ", Remaining Hours: " << plan->total_hours_remaining
                      << std::endl;
        }
    }
}

double PriceController::machine_price_computation(Machine * machine) {
    if(machine == nullptr) {
        std::cerr << "Error: can't find machine" << std::endl;
        return 0.0;
    }

    double price = 0;

    for(auto & [input, units] : machine->inputs_per_unit) {
        price += machine->price_per_unit * units;
    }

    return price;
}

void PriceController::poll_prices() {
    for (auto * firm : Society::instance->get_firms()) {
        for (auto &entry : firm->get_inventory_map()) {
            firm_price_history[firm][entry.first] = get_price(entry.first, firm);
        }
    }
}

double PriceController::product_price(Product * product, Firm * firm) {
    return get_price(product, firm);
}

