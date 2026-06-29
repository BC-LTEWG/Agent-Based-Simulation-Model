#include "Constants.h"
#include "ConsumerGood.h"
#include "Good.h"
#include "Logger.h"
#include "Sim.h"
#include "Society.h"

ConsumerGood::ConsumerGood(Good * good) :
    Product()
{
    product_type = Product::ProductType::kTypeConsumerGood;
    living_labor_per_unit *= DISTRIBUTION_COST_MULT;
    Society::get_instance()->
        set_underlying_living_labor_per_unit(this, living_labor_per_unit);
    static std::uniform_real_distribution<>
        consumption_freq_dist(1.0, PRODUCT_CONSUMPTION_VARIANCE);
    mean_consumption_frequency = consumption_freq_dist(Sim::get_random_generator());
    corresponding_good = good;
    good->corresponding_consumer_good = this;
    required_abilities = Society::get_instance()->get_distribution_abilities();
}

void ConsumerGood::set_inputs() {
    inputs_per_unit[corresponding_good] = 1.0;
}

void ConsumerGood::set_machines() {
    machines_needed = {};
}
