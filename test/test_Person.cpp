#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#define private public
#define protected public
#include "Person.h"
#undef protected
#undef private

#include "doctest.h"
#include "Firm.h"
#include "Product.h"
#include "Sim.h"
#include "Producer.h" 
#include "Society.h"
#include "ConsumerGood.h"

TEST_CASE("Person.cpp Logic Testing") {

    SUBCASE("charge() deducts money") {
        Person person;
        person.account = 100.0; //give 100
        bool success = person.charge(40.0);

        CHECK(success == true);
        CHECK(person.account == doctest::Approx(60.0)); //give 60

        success = person.charge(80.0);
        CHECK(success == false);
        CHECK(person.account == doctest::Approx(60.0)); //should still have 60
    }

    SUBCASE("productivity() scales") {
        Person person;
        person.health_status = Person::HEALTHY;
        CHECK(person.productivity() == doctest::Approx(1.0)); //healthy = 1
        person.health_status = Person::UNHEALTHY;
        CHECK(person.productivity() == doctest::Approx(UNHEALTHY_PRODUCTIVITY)); //unhealthy != 1
    }
    
    SUBCASE("set_firm() updates pointer") {
        Person person;
        CHECK(person.get_firm() == nullptr); //should return null bc unemployed

        Firm* test_firm = new Producer(); //creates firm
        person.set_firm(test_firm);
        CHECK(person.get_firm() == test_firm);
    }

    SUBCASE("train() updates ability scores") {
        Person person;
        Ability* test_ability = new Ability();
        std::unordered_map<Ability*, double> target_training;
        target_training[test_ability] = 35.0;
        person.train(target_training);

        CHECK(person.get_abilities()[test_ability] == doctest::Approx(35.0));
    }

    SUBCASE("update_busyness() applies average") {
        Person person;
        person.busyness = 10.0;
        person.busyness_this_time_step = 8.0;
        person.update_busyness();

        CHECK(person.busyness_this_time_step == doctest::Approx(0.0)); //reset here
        CHECK(person.busyness != doctest::Approx(10.0)); //must be different from init
    }

    SUBCASE("suitability() averages") {
        Person person;
        Ability* ab1 = new Ability();
        Ability* ab2 = new Ability();
        person.abilities[ab1] = 10.0;
        person.abilities[ab2] = 20.0;
        std::vector<Ability*> required_skills = {ab1, ab2};
        person.health_status = Person::HEALTHY;
        double healthy_score = person.suitability(required_skills);

        CHECK(healthy_score == doctest::Approx(15.0)); //average should be 15

        person.health_status = Person::UNHEALTHY;
        double sick_score = person.suitability(required_skills);
        CHECK(sick_score == doctest::Approx(15.0 * UNHEALTHY_PRODUCTIVITY)); //should be 15 * unhealthy
    }

    SUBCASE("consume() reduces inventory") {
        Person person; //new person 
        ConsumerGood* test_good = Society::get_instance()->get_consumer_goods()[0]; //take consumer good
        test_good->mean_consumption_frequency = 1.0;
        person.inventory[test_good] = 10;

        person.consume();
        CHECK(person.inventory[test_good] == 9);
    }

    SUBCASE("shop() returns budget") {
        Person person;
        person.account = 0.0;
        person.shop();
        for (auto const& [good, quantity] : person.inventory) { //since 0 money, inventory should be stagnant
            CHECK(quantity >= 0); 
        }
    }
}