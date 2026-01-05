#pragma once

#include <Eigen/Dense>
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
        std::vector<Person *>& get_unemployed_people();
        void retire_person(Person * person);
        int get_current_work_hours_daily();
		int get_current_work_days_weekly();
    private:
        Person * birth_person();
        void set_initial_products();
        void set_product_prices();
        std::unordered_map<Product *, std::size_t> get_product_to_index_map();
        void populate_io_matrix_and_labor_vector(
                std::unordered_map<Product *, std::size_t>&,
                Eigen::MatrixXd&,
                Eigen::VectorXd&
                );
        void adjust_io_matrix(Eigen::MatrixXd&, double max_eigenvalue);
        std::vector<Person *> people;
        std::vector<Firm *> firms;
        std::vector<Producer *> producers;
        std::unordered_map<Product *, std::vector<Distributor *>>
            product_to_distributors;
        std::vector<Product *> products;
        std::unordered_map<Product *, std::size_t> product_to_index;
        std::vector<Machine *> machines;
        std::vector<Distributor *> distributors;
        int current_work_hours_daily = INITIAL_WORK_HOURS_DAILY;
		int current_work_days_weekly = INITIAL_WORK_DAYS_WEEKLY;
        std::vector<Person *> unemployed_people;
};
