#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <unordered_map>

#include "doctest.h"
#include "Person.h"
#include "Society.h"

TEST_SUITE_BEGIN("Person");

TEST_CASE("Society exists") {
    Society * society = new Society;
    CHECK(society);
    CHECK(Society::instance);
}

TEST_CASE("Person::abilities exists") {
    Person person;
    CHECK(person.get_abilities().size());
}

TEST_SUITE_END();
