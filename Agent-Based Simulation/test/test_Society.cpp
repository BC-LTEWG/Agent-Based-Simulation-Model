#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <iostream>
#include <vector>

#include "doctest.h"
#include "Product.h"
#include "Society.h"

TEST_SUITE_BEGIN("Person");

TEST_SUITE_END();

TEST_CASE("Society exists") {
    Society * society = Society::get_instance();
    CHECK(society);
}

TEST_CASE("Constructor creates initial people") {
    std::vector<Person *> people = Society::get_instance()->get_unemployed_people();
    CHECK(people.size() == STARTING_NUM_PEOPLE);
}

TEST_CASE("Constructor creates initial distributors") {
    std::vector<Distributor *> distributors =
        Society::get_instance()->get_distributors();
    CHECK(distributors.size() == STARTING_NUM_DISTRIBUTORS);
}

TEST_CASE("Constructor creates initial products") {
    std::vector<Product *> products = Society::get_instance()->get_products();
    CHECK(products.size() == STARTING_NUM_PRODUCTS + STARTING_NUM_MACHINES);
}

TEST_CASE("Constructor sets positive prices") {
    std::vector<Product *> products = Society::get_instance()->get_products();
    for (Product * product : products) {
        CHECK(product->price_per_unit > 0.0);
    }
}

TEST_SUITE_END();

