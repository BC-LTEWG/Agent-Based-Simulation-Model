#include <iostream>

#include "Constants.h"
#include "Distributor.h"
#include "Firm.h"
#include "Machine.h"
#include "PriceController.h"
#include "Product.h"
#include "Sim.h"

PriceController * PriceController::get_instance() {
    static PriceController * instance = new PriceController;
    return instance;
}

PriceController::PriceController() {}

void PriceController::update_price(Plan * plan) {
    Product * product = plan->order->product;
    int now = Sim::get_current_time_step();
    int end_time = now - PRICE_AVERAGING_WINDOW;
    if (plan_history[product].begin()->second <= end_time) {
        plan_history[product].erase(plan_history[product].begin());
    }
    int past_total_units = 0;
    int num_workers = 0;
    for (std::pair<Plan *, int> entry : plan_history[product]) {
        past_total_units += entry.first->order->quantity;
        num_workers += entry.first->workers.size();
    }
    int total_units = past_total_units + plan->order->quantity;
    num_workers += plan->workers.size();
    double past_total_labor = product->living_labor_per_unit * past_total_units;
    double new_actual_labor =
        plan->labor_hours - plan->labor_hours_remaining;
    double average_labor_per_unit =
        (new_actual_labor + past_total_labor) / total_units;
    plan_history[product].push_back(std::make_pair(plan, now));
    double price = product->living_labor_per_unit = average_labor_per_unit;
    for (std::pair<Product *, double> input : product->inputs_per_unit) {
        price += input.first->price_per_unit * input.second;
    }
    for (Machine * machine : product->machines_needed) {
        price += (machine->price_per_unit / machine->lifetime) * average_labor_per_unit;
    }
    product->price_per_unit = price;
}

