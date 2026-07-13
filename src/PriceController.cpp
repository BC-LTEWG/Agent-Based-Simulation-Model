#include <unordered_map>

#include "Constants.h"
#include "ConsumerGood.h"
#include "Distributor.h"
#include "Firm.h"
#include "Good.h"
#include "Logger.h"
#include "Machine.h"
#include "Order.h"
#include "Plan.h"
#include "PriceController.h"
#include "Product.h"
#include "Sim.h"

PriceController::PriceController() {}

PriceController * PriceController::get_instance() {
    static PriceController * instance = new PriceController;
    return instance;
}

void PriceController::on_time_step() {
    update_fic();
}

unsigned int PriceController::get_id() {
    return 0;
}

void PriceController::update_price(Plan * plan) {
    Product * product = plan->order->product;
    int now = Sim::get_current_time_step();
    int end_time = now - PRICE_AVERAGING_WINDOW;
    if (plan_history.count(product)) {
        while (!plan_history[product].empty() &&
                plan_history[product].front().second <= end_time) {
            Plan * old_plan = plan_history[product].front().first;
            plan_history[product].pop_front();
            delete old_plan->order;
            delete old_plan;
        }
    }
    plan_history[product].push_back(std::make_pair(plan, now));
    int units = 0.0;
    double hours = 0.0;
    std::unordered_map<Product *, double> inputs_used;
    for (std::pair<Plan *, int> entry : plan_history[product]) {
        Plan * plan = entry.first;
        units += plan->order->quantity - plan->quantity_remaining;
        hours += plan->labor_hours_used;
        for (std::pair<Product *, double> input : plan->outlays) {
            inputs_used[input.first] += input.second;
        }
    }
    if (units <= 0) {
        throw std::runtime_error("Units cannot be 0 or less for product: " + product->product_name); 
    }
    if (workers <= 0) {
        throw std::runtime_error("Plan cannot be completed without workers: " + product->product_name); 
    }
    double price = product->living_labor_per_unit = hours / units;
    double inputs_cost = 0.0;
    for (std::pair<Product *, double> input : inputs_used) {
        inputs_cost += input.second * input.first->price_per_unit;
    }
    price += inputs_cost / units;
    product->price_per_unit = price;
    Logger::log(
            Logger::SOCIETY,
            0,
            "new_price",
            LogPair("product_id", product->id),
            LogPair("price", price)
            );
}

void PriceController::report_distribution(ConsumerGood * consumer_good, int quantity) {
    double added_value = 
        consumer_good->price_per_unit
        * quantity
        / FIC_AVERAGING_WINDOW;
    consumer_good_to_net_value[consumer_good] += added_value;
}

double PriceController::get_fic() {
    return fic;
}

void PriceController::update_fic() {
    double public_sector_net_value = 0.0;
    double societal_net_value = 0.0;
    for (std::pair<ConsumerGood * const, double>& consumer_good : consumer_good_to_net_value) {
        double decay = consumer_good.second / FIC_AVERAGING_WINDOW;
        consumer_good.second -= decay;
        societal_net_value += consumer_good.second; 
        if (consumer_good.first->public_sector) {
            public_sector_net_value += consumer_good.second;
        }
    }
    log_public_sector_net_value(public_sector_net_value);
    log_societal_net_value(societal_net_value);
    if (societal_net_value > 0.0) {
        fic = 1.0 - public_sector_net_value / societal_net_value;
    } else {
        fic = 1.0;
    }
    log_fic();
}

void PriceController::log_public_sector_net_value(double value) {
    Logger::log(
            Logger::SOCIETY,
            get_id(),
            "public_sector_net_value",
            LogPair("value", value)
            );
}

void PriceController::log_societal_net_value(double value) {
    Logger::log(
            Logger::SOCIETY,
            get_id(),
            "societal_net_value",
            LogPair("value", value)
            );
}

void PriceController::log_fic() {
    Logger::log(
            Logger::SOCIETY,
            get_id(),
            "fic",
            LogPair("value", fic)
            );
}

