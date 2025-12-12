#pragma once

#include <unordered_map>

#include "Constants.h"

struct Product;
class Distributor;
class Firm;
struct Machine;
class Person;
class Producer;

class Society {
    public:
        Society();
        static Society * instance;
        std::vector<Product *>& get_products();
        std::vector<Distributor *>& get_distributors();
        void retire_person(Person * person);


        std::size_t num_people();
        std::size_t num_firms();
        bool meets_standard_for_lower_working_hours();
        void set_work_hours_daily(int hours);
        double get_avg_productivity();
        std::unordered_map<std::string, int> avg_worker_needs();
        Person * birth_person();
        void add_unemployed(Person * person);
        std::vector<Person *> people;
        std::vector<Person *> unemployed_people;
        std::vector<Firm *> firms;
        std::vector<Producer *> producers;
        std::unordered_map<Product *, std::vector<Distributor *>> product_to_distributors;
        std::unordered_map<Firm *, double> prices;
        std::unordered_map<std::string, int> avg_needs();
        int current_work_hours_daily = INITIAL_WORK_HOURS_DAILY;
    private:
        std::vector<Product *> products;
        std::vector<Distributor *> distributors;
};
