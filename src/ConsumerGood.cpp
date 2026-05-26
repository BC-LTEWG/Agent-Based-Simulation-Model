#include "Constants.h"
#include "ConsumerGood.h"
#include "Good.h"
#include "Logger.h"
#include "Sim.h"
#include "Society.h"

ConsumerGood::ConsumerGood(Good * good) :
    Product()
{
    product_type = Product::ProductType::TYPE_CONSUMER_GOOD;
    living_labor_per_unit = DISTRIBUTION_LABOR_PER_UNIT;
    Society::get_instance()->
        set_underlying_living_labor_per_unit(this, living_labor_per_unit);
    static std::uniform_real_distribution<>
        consumption_freq_dist(0.0, 1.0);
    mean_consumption_frequency = consumption_freq_dist(Sim::get_random_generator());
    corresponding_good = good;
    inputs_per_unit[corresponding_good] = 1.0;
    good->set_corresponding_consumer_good(this);
}

void ConsumerGood::set_inputs() {
    inputs_per_unit[corresponding_good] = 1.0;
}

void ConsumerGood::set_machines() {
    machines_needed = {};
}

void ConsumerGood::log_mean_consumption_frequency() {
    Logger::log(
            Logger::PRODUCT,
            id,
            "mean_consumption_frequency",
            LogPair("value", mean_consumption_frequency)
            );
}
