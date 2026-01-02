#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <random>

#include "Constants.h"
#include "Firm.h"
#include "Distributor.h"
#include "Person.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Person::Person():
    age(INITIAL_AGE),
    health_status(HEALTHY) {
        static std::normal_distribution<>
            shopping_dist(
                    PERSON_SHOPPING_PERIOD / 2, PERSON_SHOPPING_OFFSET_STDDEV
                    );
        shopping_offset =
            (((int) shopping_dist(Sim::gen)) +
             PERSON_SHOPPING_PERIOD) % PERSON_SHOPPING_PERIOD;

        static std::normal_distribution<>
            ability_dist(1.0, PERSON_ABILITY_STDDEV);
        std::vector<Person::Ability> all_abilities;
        for (int i = 0; i < Person::NUM_ABILITIES; i++) {
            all_abilities.push_back((Person::Ability) i);
        }
        std::shuffle(all_abilities.begin(), all_abilities.end(), Sim::gen);
        all_abilities.resize(PERSON_ABILITY_COUNT_MAX);
        for (Person::Ability ability : all_abilities) {
            abilities[ability] = std::max(0.0, ability_dist(Sim::gen));
        }
        ranked_distributors = Society::instance->get_distributors();
        std::shuffle(
                ranked_distributors.begin(),
                ranked_distributors.end(),
                Sim::gen
                );
    shopping_offset =
        (((int) shopping_dist(Sim::gen)) +
         PERSON_SHOPPING_PERIOD) % PERSON_SHOPPING_PERIOD;

    static std::normal_distribution<>
        ability_dist(1.0, PERSON_ABILITY_STDDEV);
    std::vector<Ability> all_abilities;
    for (int i = 0; i < NUM_ABILITIES; i++) {
        all_abilities.push_back((Ability) i);
    }
    std::shuffle(all_abilities.begin(), all_abilities.end(), Sim::gen);
    all_abilities.resize(PERSON_ABILITY_COUNT_MAX);
    for (Ability ability : all_abilities) {
        abilities[ability] = std::max(0.0, ability_dist(Sim::gen));
    }
    ranked_distributors = Society::instance->get_distributors();
    std::shuffle(
            ranked_distributors.begin(),
            ranked_distributors.end(),
            Sim::gen
            );
    static std::normal_distribution<>
        dist(1, PERSON_FREQUENCY_MULTIPLIER_STDDEV);
    for (Product * p : Society::instance->get_products()) {
        purchase_frequencies[p] =
            p->mean_consumption_frequency * std::abs(dist(Sim::gen));
    }
}

std::unordered_map<Person::Ability, double>& Person::get_abilities() {
    return this->abilities;
}

void Person::train(std::unordered_map<Person::Ability, double> target_abilities) {
    // can introduce < 100% effectiveness on training later
    for (auto &pair : target_abilities) {
        abilities[pair.first] = pair.second;
    }
}

void Person::register_hours_worked(double hours_worked) {
    account += hours_worked;
}

void Person::charge(double cost) {
    account -= cost; 
}

std::unordered_map<Product*, double>& Person::get_purchase_frequencies() { 
    return this->purchase_frequencies;
}

Person::HealthStatus Person::get_health_status() {
    return this->health_status;
}

float Person::get_current_productivity() {
    switch(health_status) {
        case HEALTHY:
            return 1.0;
        case UNHEALTHY:
            return UNHEALTHY_PRODUCTIVITY;
        default:
            return 1.0;
    }
}

float Person::avg_productivity_over_time_step(std::string product_name) {
    (void)product_name;
    return 0.0f;
}

void Person::purchase_good(Product * p, int quantity) {
    for (Distributor * distributor : ranked_distributors) {
        if (distributor->has_product(p)) {
            distributor->sell_goods(*p, quantity, this);
            return;
        }
    }
}

bool Person::will_shop() {
    return Sim::get_current_time_step() % PERSON_SHOPPING_PERIOD ==
        shopping_offset;
}

void Person::shop() {
    static std::normal_distribution<>
        dist(1, PERSON_SHOPPING_MULTIPLIER_STDDEV);
    for (auto &p : purchase_frequencies) {
        int quantity = std::round(p.second * PERSON_SHOPPING_PERIOD *
                std::abs(dist(Sim::gen)));
        if (quantity > 0) {
            purchase_good(p.first, quantity);
        }
    }
}

bool Person::will_retire() {
    static std::uniform_real_distribution<> dist(0, 1);
    if (age >= GUARANTEED_RETIREMENT_AGE) { return true; }
    return dist(Sim::gen) < RANDOM_RETIREMENT_CHANCE;
}

void Person::retire() {
    Society::instance->retire_person(this);
}

void Person::update_health_status() {
    static std::uniform_real_distribution<> dist(0, 1);
    if (health_status == HEALTHY &&
            dist(Sim::gen) < 1 - pow(1 - DAILY_SICKNESS_CHANCE, 1.0 / DAY)) {
        health_status = UNHEALTHY;
    } else if (health_status == UNHEALTHY &&
            dist(Sim::gen) < 1 - pow(1 - DAILY_RECOVERY_CHANCE, 1.0 / DAY)) {
        health_status = HEALTHY;
    } 
}


void Person::on_time_step() {
    ++age;
    if (will_shop()) { shop(); }
    if (will_retire()) { retire(); }
    update_health_status();
}

void Person::set_firm(Firm * workplace) {
    firm = workplace;
}
