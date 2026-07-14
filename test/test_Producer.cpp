#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#define private public
#define protected public
#include "Society.h" 
#include "Producer.h"
#include "Firm.h"
#undef protected
#undef private

#include "doctest.h"
#include "Product.h"
#include "Good.h"
#include "Machine.h"
#include "Logger.h"
#include <vector>
#include <climits>
#include <algorithm>

TEST_CASE("Producer Logic Testing") {
    std::vector<Producer*> producers = Society::get_instance()->get_producers(); //find a factory

    REQUIRE(producers.size() > 0);
    Producer* test_producer = producers[0];

    REQUIRE(test_producer->catalog.empty() == false);
    Product* test_product = *(test_producer->catalog.begin());

    SUBCASE("get_client_type() identifies producer") {
        CHECK(test_producer->get_client_type() == Logger::PRODUCER);
    }

    SUBCASE("add_to_catalog() calculates demand and inventory") { //initial stock > 0
        for (Machine* machine : test_product->machines_needed) {
            CHECK(test_producer->machines.count(machine) > 0);
        }
        for (std::pair<Good * const, double>& input : test_product->inputs_per_unit) {
            Good* input_good = input.first;
            CHECK(test_producer->input_inventory[input_good] > 0);
        }
    }

    SUBCASE("can_produce() checks producer") {
        CHECK(test_producer->can_produce(test_product) == true);
    }

    SUBCASE("get_max_order_quantity() calculates limits correctly") {
        int expected_max_quantity = INT_MAX;
        for (std::pair<Good * const, double>& input : test_product->inputs_per_unit) { //manually calculate actual value
            double current_inventory = test_producer->input_inventory[input.first];
            double required_per_unit = input.second;
            int input_max = static_cast<int>(current_inventory / required_per_unit);
            expected_max_quantity = std::min(expected_max_quantity, input_max); 
        }
        CHECK(test_producer->get_max_order_quantity(test_product) == expected_max_quantity); //projected vs actual
    }
}