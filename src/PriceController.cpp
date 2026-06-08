#include "Constants.h"
#include "ConsumerGood.h"
#include "Distributor.h"
#include "Firm.h"
#include "Good.h"
#include "Logger.h"
#include "Machine.h"
#include "PriceController.h"
#include "Product.h"
#include "Sim.h"

PriceController::PriceController() {}

PriceController * PriceController::get_instance() {
    static PriceController * instance = new PriceController;
    return instance;
}

void PriceController::on_time_step() {
    double decay = 1.0 - 1.0 / FIC_AVERAGING_WINDOW;
    paid_consumer_goods_value *= decay;
    all_consumer_goods_value *= decay;
    fic = 1.0;
    if (all_consumer_goods_value) {
        fic = paid_consumer_goods_value / all_consumer_goods_value;
    }
}

unsigned int PriceController::get_id() {
    return 0;
}

void PriceController::update_price(Plan * plan) {
    Product * product = plan->order->product;
    int now = Sim::get_current_time_step();
    int end_time = now - PRICE_AVERAGING_WINDOW;
    if (plan_history.count(product) && 
            plan_history[product].begin()->second <= end_time) {
        plan_history[product].erase(plan_history[product].begin());
    }
    plan_history[product].push_back(std::make_pair(plan, now));
    int units = 0.0;
    double hours = 0.0;
    int workers = 0;
    for (std::pair<Plan *, int> entry : plan_history[product]) {
        Plan * plan = entry.first;
        units += plan->order->quantity - plan->quantity_remaining;
        hours += plan->labor_hours - plan->labor_hours_remaining;
        workers += plan->workers.size();
    }
    double price = product->living_labor_per_unit = hours / units;
    double machine_use_hours = hours / workers;
    double machine_hours_per_unit = machine_use_hours / units;
    for (std::pair<Good * const, double>& input_pair : product->inputs_per_unit) {
        double input_quantity_per_unit = input_pair.second;
        price += input_pair.first->price_per_unit * input_quantity_per_unit;
    }
    for (Machine * machine : product->machines_needed) {
        double machine_cost_per_hour =
            machine->price_per_unit / machine->lifetime;
        price += machine_cost_per_hour * machine_hours_per_unit;
    }
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
    all_consumer_goods_value += added_value;
    if (consumer_good->paid) {
        paid_consumer_goods_value += added_value;
    }
}

double PriceController::get_fic() {
    return fic;
}

