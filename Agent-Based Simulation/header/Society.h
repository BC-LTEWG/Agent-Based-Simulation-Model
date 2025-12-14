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
        std::vector<Machine *>& get_machines();
        std::vector<Distributor *>& get_distributors();
        std::vector<Producer *>& get_producers();
        std::vector<Firm *>& get_firms();
        std::vector<Person *>& get_unemployed_people();
        void retire_person(Person * person);
        int get_current_work_hours_daily();
    private:
        Person * birth_person();
        void set_initial_products();
        void set_initial_machines();
        std::vector<Person *> people;
        std::vector<Firm *> firms;
        std::vector<Producer *> producers;
        std::unordered_map<Product *, std::vector<Distributor *>> product_to_distributors;
        std::vector<Product *> products;
        std::vector<Machine *> machines;
        std::vector<Distributor *> distributors;
        int current_work_hours_daily = INITIAL_WORK_HOURS_DAILY;
        std::vector<Person *> unemployed_people;
};
