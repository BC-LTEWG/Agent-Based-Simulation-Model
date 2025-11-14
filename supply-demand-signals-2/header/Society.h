#pragma once

#include <unordered_map>

#include "Distributor.h"
#include "Firm.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"

class Firm;
class Person;

class Society{
  public:
    std::vector<Person*> people;
	std::vector<Person*> unemployed_people;
    std::vector<Firm*> firms;
	std::vector<Product*> products;
	std::vector<Producer*> producers;
	std::vector<Distributor*> distributors;
	std::unordered_map<Product*, Distributor*> product_to_distributor;
    std::unordered_map<Firm*, double> prices;
    std::unordered_map<std::string, int> avg_needs();
    const int initial_work_hours_daily = 8;
    int current_work_hours_daily = initial_work_hours_daily;

    Society(std::vector<Person*> people, std::vector<Product*> products, std::vector<Producer*> producers, std::vector<Distributor*> distributors, std::unordered_map<Product*, Distributor*> product_to_distributor, std::unordered_map<Firm*, double> prices);

    std::size_t num_people();
    std::size_t num_firms();
    bool meets_standard_for_lower_working_hours();
    void set_work_hours_daily(int hours);
    double get_avg_productivity();
    std::unordered_map<std::string, int> avg_worker_needs();
	Person& birth_worker();
	void birth_people_every_timestep();
	void retire_person(Person& person);
};
