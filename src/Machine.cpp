#include <string>

#include "Machine.h"
#include "Sim.h"

Machine::Machine() : Product() {
    product_type = Product::ProductType::kTypeMachine;
    static std::uniform_int_distribution<>
        machine_lifetime_dist(MACHINE_LIFETIME_MIN, MACHINE_LIFETIME_MAX);
    lifetime = machine_lifetime_dist(Sim::get_random_generator());
}

