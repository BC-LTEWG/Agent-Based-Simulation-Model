#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "Machine.h"
#include "Society.h"
#include "Sim.h"
#include "Constants.h"
#include <vector>

TEST_CASE("Machine Logic Testing") {
    std::vector<Machine*> machines = Society::get_instance()->get_machines(); //find array of machines
    REQUIRE(machines.size() > 0); 
    Machine* test_machine = machines[0]; 

    SUBCASE("Product type is assigned") {
        CHECK(test_machine->product_type == Product::ProductType::TYPE_MACHINE);
    }

    SUBCASE("Machine lifetime within bounds") {
        CHECK(test_machine->lifetime >= MACHINE_LIFETIME_MIN);
        CHECK(test_machine->lifetime <= MACHINE_LIFETIME_MAX);
    }
}