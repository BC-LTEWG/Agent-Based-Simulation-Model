#include <iostream>

#include "Constants.h"
#include "Distributor.h"
#include "Firm.h"
#include "Logger.h"
#include "Machine.h"
#include "PriceController.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

PriceController * PriceController::get_instance() {
    static PriceController * instance = new PriceController;
    return instance;
}

PriceController::PriceController() {}

void PriceController::update_price(Plan * plan) {
    Product * product = plan->order->product;
    int now = Sim::get_current_time_step();
    int end_time = now - PRICE_AVERAGING_WINDOW;
    if (plan_history.count(product) && 
            plan_history[product].begin()->second <= end_time) {
        plan_history[product].erase(plan_history[product].begin());
    }
    plan_history[product].push_back(std::make_pair(plan, now));
    double units = 0.0;
    double hours = 0.0;
    int workers = 0;
    for (std::pair<Plan *, int> entry : plan_history[product]) {
        Plan * plan = entry.first;
        units += plan->order->quantity - plan->quantity_remaining;
        hours += plan->labor_hours - plan->labor_hours_remaining;
        workers += plan->workers.size();
    }
    double price = product->living_labor_per_unit = hours / units;
    for (std::pair<Product *, double> input : product->inputs_per_unit) {
        price += input.first->price_per_unit * input.second;
    }
    // std::cerr << "Updated living labor per unit plus cost of inputs is: " << price << std::endl;
    double machine_use_hours = hours / workers;
    double machine_hours_per_unit = machine_use_hours / units;
    for (Machine * machine : product->machines_needed) {
        double machine_cost_per_hour =
            machine->price_per_unit / machine->lifetime;
        price += machine_cost_per_hour * machine_hours_per_unit;
    }
    product->price_per_unit = price;
    // std::cerr << "Updating per-unit price to " << price << std::endl;
    Logger::get_instance()->log(Logger::SOCIETY, "price", product->id, price);
}

