#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "Good.h"
#include "Society.h"
#include "Product.h"

TEST_CASE("Good Logic Testing") {
    Good* test_good = nullptr; //find a "good"
    for (Product* p : Society::get_instance()->get_products()) {
        if (p->product_type == Product::ProductType::TYPE_GOOD) {
            test_good = static_cast<Good*>(p);
            break; 
        }
    }
    
    REQUIRE(test_good != nullptr); //"good" must be here

    SUBCASE("Product type is assigned") {
        CHECK(test_good->product_type == Product::ProductType::TYPE_GOOD); //constructor recognizes
    }
}