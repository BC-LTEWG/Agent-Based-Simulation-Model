#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#define private public
#define protected public

#include "Product.h"
#include "Good.h"
#include "Machine.h"
#include "Society.h"
#include "Sim.h"
#include "Constants.h"
#undef protected
#undef private
#include "doctest.h"

TEST_CASE("Product Logic Testing") {

    Product* p = Society::get_instance()->get_products()[0]; //find product

    SUBCASE("Constructor has labor") {
        CHECK(p->living_labor_per_unit >= PRODUCT_LABOR_PER_UNIT_MIN); //check value exists
        CHECK(p->living_labor_per_unit <= PRODUCT_LABOR_PER_UNIT_MAX);

        CHECK(p->required_abilities.size() > 0);
        CHECK(p->required_abilities.size() <= PRODUCT_ABILITY_COUNT_MAX);
    }

    SUBCASE("set_inputs() values equal recipes") {
        CHECK(p->inputs_per_unit.size() >= (size_t)PRODUCT_NUM_INPUTS_MIN); //# of inputs/ingredients > # of products
    }

    SUBCASE("set_machines() assigns tools") {
        if (Society::get_instance()->get_machines().size() > 0) {
            CHECK(p->machines_needed.size() >= (size_t)PRODUCT_NUM_MACHINES_MIN); //machinery assigns to each item
        }
    }
}