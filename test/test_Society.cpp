#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#define private public
#define protected public
#include "Society.h" 
#include "Person.h"
#undef protected
#undef private

#include "Good.h"
#include "ConsumerGood.h"
#include "Sim.h"
#include "doctest.h"

TEST_CASE("Society Logic Testing") {
    Society* society = Society::get_instance();
    
    SUBCASE("set_abilities distributes and randomly sizes") {
        std::vector<Ability*> test_abilities;
        std::vector<Ability*> test_dist_abilities;
        society->set_abilities(test_abilities, test_dist_abilities);

        CHECK(test_abilities.size() == Sim::get_num_abilities());
        CHECK(test_dist_abilities.size() > 0);
        CHECK(test_dist_abilities.size() <= PRODUCT_ABILITY_COUNT_MAX);
    }

    SUBCASE("set_initial_account calculates correct baseline money") {
        double test_account = 0.0;
        Good* test_good = new Good(); 
        ConsumerGood* test_consumer = new ConsumerGood(test_good);
        test_consumer->price_per_unit = 10.0;
        test_consumer->mean_consumption_frequency = 2.0;

        std::vector<ConsumerGood*> test_goods = {test_consumer};
        society->set_initial_account(test_account, test_goods);

        double expected_math = (10.0 * 2.0) * INITIAL_ACCOUNT_DURATION;
        CHECK(test_account == doctest::Approx(expected_math));
    }

    SUBCASE("get_instance has singleton pattern") {
        Society* second_instance = Society::get_instance();
        CHECK(society != nullptr); //points to something that actually exists
        CHECK(society == second_instance); //if false, singleton is false (must be same address)
    }

    SUBCASE("get_id() returns 0") {
        CHECK(society->get_id() == 0);
        society->id = 1;
        CHECK_THROWS_AS(society->get_id(), std::invalid_argument);
        society->id = 0; 
    }

    SUBCASE("on_time_step() gets average") {
        if (society->people.empty()) {
            society->birth_person();
            society->birth_person();
            society->birth_person();
        }
        society->on_time_step(); //simulation here

        double expected_busyness = 0.0;
        double expected_account = 0.0;
        for (Person* p : society->people) {
            expected_busyness += p->get_busyness();
            expected_account += p->get_account();
        }
        expected_busyness /= society->people.size();
        expected_account /= society->people.size();

        CHECK(society->busyness == doctest::Approx(expected_busyness));
        CHECK(society->average_account == doctest::Approx(expected_account).epsilon(0.001));
    }

    SUBCASE("get_total_employment returns percentage") {
        double employment_rate = society->get_total_employment(); //between 0 - 1
        CHECK(employment_rate >= 0.0);
        CHECK(employment_rate <= 1.0);
    }

    SUBCASE("public_fund adds/subtracts") {
        double base = society->public_fund;
        society->pay_into_public_fund(100.0);
        CHECK(society->public_fund == doctest::Approx(base + 100.0));
    
        society->charge_from_public_fund(50.0);
        CHECK(society->public_fund == doctest::Approx(base + 50.0));
    }

    SUBCASE("set_initial_products generates properly") {
        unsigned int total_goods = society->goods.size(); //take the values
        unsigned int total_consumers = society->consumer_goods.size();
        unsigned int total_machines = society->machines.size(); //expected value
        unsigned int master_total = society->products.size(); //expected value

        CHECK(total_goods > 0);
        CHECK(total_consumers > 0);
        CHECK(total_machines > 0);
        
        size_t expected_total = total_goods + total_consumers + total_machines; //actual value
        CHECK(master_total == expected_total);

        unsigned int expected_machines = Sim::get_num_machines();
        CHECK(total_machines == expected_machines);
    }

    SUBCASE("birth_person increments population") {
        int initial_population = society->people.size();
        int initial_unemployed = society->unemployed_people.size();

        Person* new_baby = society->birth_person(); //create new object
        CHECK(new_baby != nullptr); //did it create?
        CHECK(society->people.size() == initial_population + 1); //check
        CHECK(society->unemployed_people.size() == initial_unemployed + 1); 
    }

    SUBCASE("populate_io_matrix_and_labor_vector structures") {
        size_t dim = society->get_products().size(); //size of economy
        Eigen::MatrixXd test_matrix = Eigen::MatrixXd::Zero(dim, dim); //matrix
        Eigen::VectorXd test_vector = Eigen::VectorXd::Zero(dim);

        society->populate_io_matrix_and_labor_vector(test_matrix, test_vector); //call
        double total_labor = test_vector.sum();
        CHECK(total_labor > 0.0); //check actual input onto matrix
    }
}