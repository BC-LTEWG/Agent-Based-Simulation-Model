#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "ConsumerGood.h"
#include "Good.h"
#include "Sim.h"
#include "Constants.h"
#include "Society.h"

TEST_CASE("ConsumerGood Logic Testing") {
    ConsumerGood* test_cg = nullptr; //find consumergood
    for (Product* p : Society::get_instance()->get_products()) {
        if (p->product_type == Product::ProductType::TYPE_CONSUMER_GOOD) {
            test_cg = static_cast<ConsumerGood*>(p);
            break;
        }
    }
    
    REQUIRE(test_cg != nullptr); //must exist

    SUBCASE("Product type is assigned") {
        CHECK(test_cg->product_type == Product::ProductType::TYPE_CONSUMER_GOOD);
    }

    SUBCASE("Pointers are mapped") {
        REQUIRE(test_cg->corresponding_good != nullptr); // good/consumer knows what its being turned into/from
        CHECK(test_cg->corresponding_good->corresponding_consumer_good == test_cg);
    }

    SUBCASE("Consumption frequency works") {
        CHECK(test_cg->mean_consumption_frequency > 0); //frequency exists
    }

    SUBCASE("Theoretical basis for consumption frequencies (issue #144)") { //checking dot product of lambda & consumption frequency
        double labor_value_consumed = 0;
        for(Product* p : Society::get_instance()->get_products()) {
            if(p->product_type == Product::ProductType::TYPE_CONSUMER_GOOD) {
                ConsumerGood* test_good = static_cast<ConsumerGood*>(p);
                double test_lambda = test_good->labor_value; //find labor value (lambda)
                double test_frequency = test_good->mean_consumption_frequency; //find avg person consume in 1 hr

                labor_value_consumed += (test_lambda * test_frequency); // λ . c < 1
            }
        }
        CHECK(labor_value_consumed < 1);
    }
}