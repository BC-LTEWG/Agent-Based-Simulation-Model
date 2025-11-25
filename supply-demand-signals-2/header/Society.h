#pragma once

#include <unordered_map>

#include "Constants.h"
#include "Distributor.h"
#include "Firm.h"
#include "Machine.h"
#include "Person.h"
#include "Producer.h"

class Firm;
class Person;

class Society{
  public:
    Society(std::vector<Person *> people, std::vector<Product *> products, std::vector<Producer *> producers, std::vector<Distributor *> distributors, std::unordered_map<Product *, std::vector<Distributor *>> product_to_distributors, std::unordered_map<Firm *, double> prices);

    std::size_t num_people();
    std::size_t num_firms();
    bool meets_standard_for_lower_working_hours();
    void set_work_hours_daily(int hours);
    double get_avg_productivity();
    std::unordered_map<std::string, int> avg_worker_needs();
	Person& birth_worker();
	void birth_people_every_timestep();
	void retire_person(Person& person);

    // Getters for data members
    const std::vector<Person *>& get_people() const;
    const std::vector<Person *>& get_unemployed_people() const;
    const std::vector<Firm *>& get_firms() const;
    const std::vector<Product *>& get_products() const;
    const std::vector<Producer *>& get_producers() const;
    const std::vector<Distributor *>& get_distributors() const;
    const std::unordered_map<Product *, std::vector<Distributor *>>& get_product_to_distributors() const;
    const std::unordered_map<Firm *, double>& get_prices() const;
    int get_current_work_hours_daily() const;

  private:
    std::vector<Person *> people;
	std::vector<Person *> unemployed_people;
    std::vector<Firm *> firms;
	std::vector<Product *> products;
	std::vector<Producer *> producers;
	std::vector<Distributor *> distributors;
	std::unordered_map<Product *, std::vector<Distributor *>> product_to_distributors;
    std::unordered_map<Firm *, double> prices;
    int current_work_hours_daily = INITIAL_WORK_HOURS_DAILY;
};
