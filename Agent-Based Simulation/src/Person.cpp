#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <random>

#include "Constants.h"
#include "ConsumerGood.h"
#include "Distributor.h"
#include "Firm.h"
#include "Logger.h"
#include "Person.h"
#include "Product.h"
#include "Sim.h"
#include "Society.h"

Person::Person(Society * society):
    society{society},
    age(INITIAL_AGE),
    health_status(HEALTHY)
{
    static unsigned int unique_id = 0;
    id = unique_id++;

    static std::normal_distribution<>
        ability_dist(1.0, PERSON_ABILITY_STDDEV);
    std::vector<Person::Ability> variated_abilities;
    for (int i = 0; i < Person::NUM_ABILITIES; i++) {
        abilities[(Person::Ability) i] = 1.0;
        variated_abilities.push_back((Person::Ability) i);
    }
    std::shuffle(variated_abilities.begin(), variated_abilities.end(), Sim::get_random_generator());
    for (int i = 0; i < PERSON_VARIATED_ABILITY_COUNT; i++) {
        abilities[variated_abilities[i]] = std::max(0.0, ability_dist(Sim::get_random_generator()));
    }
    ranked_distributors = society->get_distributors();
    std::shuffle(
            ranked_distributors.begin(),
            ranked_distributors.end(),
            Sim::get_random_generator()
            );
    for (Product * product : society->get_goods()) {
        inventory[product] = 
            (int) (PERSON_STOCKPILE_DURATION * product->mean_consumption_frequency);
    }
    account = society->get_initial_account();
}

unsigned int Person::get_id() {
    return id;
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
    log_hours(hours_worked);
    account += hours_worked;
}

bool Person::charge(double cost) {
    if (cost > account) {
        return false;
    }
    account -= cost; 
    return true;
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

void Person::purchase_good(Product * p, int quantity) {
    for (Distributor * distributor : ranked_distributors) {
        if (distributor->try_sell_goods(*p, quantity, this)) {
            inventory[p] += quantity;
            return;
        }
    }
}

void Person::consume() {
    for (Product * product : society->get_goods()) {
        inventory[product] -= 
            (int) (Sim::get_current_time_step() * product->mean_consumption_frequency) - 
            (int) ((Sim::get_current_time_step() - 1) * product->mean_consumption_frequency);
    }
}

bool Person::will_shop() {
    double total_deficit = 0.0;
    for (Product * product : society->get_goods()) {
        total_deficit += std::max(0.0, 
            PERSON_STOCKPILE_DURATION - 
            inventory[product] / product->mean_consumption_frequency
        );
    }
    return total_deficit > PERSON_DEFICIT_THRESHOLD;
}

void Person::shop() {
    double total_price = 0.0;
    static std::unordered_map<Product *, int> purchase_quantities;
    for (Product * product : society->get_goods()) {
        purchase_quantities[product] = std::max(0, 
            (int) (PERSON_STOCKPILE_DURATION * product->mean_consumption_frequency) - 
            inventory[product]
        );
        total_price += purchase_quantities[product] * 
            society->get_consumer_good(product)->price_per_unit;
    }
    double price_scalar = std::min(account / total_price, 1.0);
    log_shopping_deficit(std::max(0.0, 1.0 - price_scalar)); 
    for (std::pair<Product *, int> p : purchase_quantities) {
        int quantity = (int) (price_scalar * p.second);
        if (quantity > 0) {
            purchase_good(p.first, quantity);
            log_shopping(p.first->product_name, quantity);
        }
    }
}

bool Person::will_retire() {
    static std::uniform_real_distribution<> dist(0, 1);
    if (age >= GUARANTEED_RETIREMENT_AGE) { return true; }
    return dist(Sim::get_random_generator()) < RANDOM_RETIREMENT_CHANCE;
}

void Person::retire() {
    society->retire_person(this);
}

void Person::update_health_status() {
	static std::uniform_real_distribution<> dist(0, 1);
	if (health_status == HEALTHY &&
		dist(Sim::get_random_generator()) < 1 - pow(1 - DAILY_SICKNESS_CHANCE, 1.0 / DAY)) {
		health_status = UNHEALTHY;
	} else if (health_status == UNHEALTHY &&
	   dist(Sim::get_random_generator()) < 1 - pow(1 - DAILY_RECOVERY_CHANCE, 1.0 / DAY)) {
		health_status = HEALTHY;
	} 
}


void Person::on_time_step() {
	++age;
    consume();
	if (will_shop()) { shop(); }
	if (will_retire()) { retire(); }
	update_health_status();
    log_state();
}

void Person::set_firm(Firm * workplace) {
    firm = workplace;
}

void Person::log_hours(const double hours) {
    Logger::get_instance()->log(Logger::PERSON, "hours", id, hours);
}

void Person::log_shopping(const std::string product_name, const int quantity) {
    Logger::get_instance()->log(Logger::PERSON, "shopping", id, product_name, quantity);
}

void Person::log_shopping_deficit(const double deficit) {
    Logger::get_instance()->log(Logger::PERSON, "shopping_deficit", id, deficit);
}

void Person::log_state() {
    Logger::get_instance()->log(Logger::PERSON, "age", id, age);
    Logger::get_instance()->log(Logger::PERSON, "account", id, account);
    Logger::get_instance()->log(Logger::PERSON, "health_status", id, health_status);
}

