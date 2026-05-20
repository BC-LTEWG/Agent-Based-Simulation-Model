#include <algorithm>
#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>

#include "Constants.h"
#include "ConsumerGood.h"
#include "Distributor.h"
#include "Firm.h"
#include "Good.h"
#include "Logger.h"
#include "Machine.h"
#include "Person.h"
#include "Product.h"
#include "Producer.h"
#include "Sim.h"
#include "Society.h"

Society * Society::get_instance() {
    static Society * instance = new Society;
    if (!instance->initialized) {
        instance->initialize();
    }
    return instance;
}

Society::Society() :
    current_work_hours_daily{Sim::get_work_hours_daily()},
    current_work_days_weekly{Sim::get_work_days_weekly()}
{}

void Society::initialize() {
    initialized = true;
    set_initial_products();
    for (Product * product : products) {
        Logger::get_instance()->log(Logger::SOCIETY, "price", product->id, product->price_per_unit);
    }
    for (unsigned int i = 0; i < Sim::get_num_producers(); i++) {
        Producer * producer = new Producer(this, {goods[i % Sim::get_num_products()]});
        producers.push_back(producer);
        firms.push_back(producer);
    }
    std::unordered_set<Product *> distributor_catalog(consumer_goods.begin(), consumer_goods.end());
    for (unsigned int i = 0; i < Sim::get_num_distributors(); i++) {
        Distributor * distributor = new Distributor(this, distributor_catalog);
        distributors.push_back(distributor);
        firms.push_back(distributor);
    }
    for (Firm * firm : firms) {
        for (Producer * producer : producers) {
            firm->add_supplier(producer);
        }
    }
    set_initial_account();
    for (unsigned int i = 0; i < Sim::get_num_people(); i++) {
        birth_person();
    }
}

unsigned int Society::get_id() {
    if (id) {
        throw std::invalid_argument("Society should be a singleton.");
    }
    return id;
}

void Society::on_time_step() {
    for (Person * person : people) {
        person->on_time_step();
    }
    for (Firm * firm : firms) {
        firm->on_time_step();
    }
    // It's too early to be trying to do this, we need to put more thought into it first.
    // if (Sim::get_current_time_step() >= WORK_HOURS_UPDATE_START &&
    //         Sim::get_current_time_step() % WORK_HOURS_UPDATE_PERIOD == 0) {
    //     update_work_hours_daily();
    // }
}

void Society::set_initial_products() {
    unsigned int starting_num_products = Sim::get_num_products();
    const unsigned int starting_num_machines =
        starting_num_products / Sim::get_products_per_machine();
    for (unsigned int i = 0; i < starting_num_products; ++i) {
        Good * new_good = new Good();
        goods.push_back(new_good);
        products.push_back(new_good);
    }
    for (unsigned int i = 0; i < starting_num_products; ++i) {
        ConsumerGood * new_consumer_good = new ConsumerGood(goods[i]); 
        consumer_goods.push_back(new_consumer_good);
        products.push_back(new_consumer_good);
    }
    for (unsigned int i = 0; i < starting_num_machines; ++i) {
        Machine * new_machine = new Machine();
        machines.push_back(new_machine);
        products.push_back(new_machine);
    }
    for (Product * product : products) {
        product->set_inputs();
        product->set_machines();
    }
    set_product_prices_production_consumption();
    log_consumption_frequencies();
}

void Society::populate_io_matrix_and_labor_vector(
    Eigen::MatrixXd &input_output_matrix,
    Eigen::VectorXd &labor_vector) {
    const unsigned int starting_num_firms =
        Sim::get_num_producers() + Sim::get_num_distributors();
    const unsigned int average_team_size =
        std::max<unsigned int>(Sim::get_num_people() / starting_num_firms, 1);
    for (Product * output_product : products) {
        for (const std::pair<Good * const, double> &input :
             output_product->inputs_per_unit) {
            input_output_matrix(input.first->id, output_product->id) = input.second;
        }
        double machine_use_hours =
            output_product->living_labor_per_unit / average_team_size;
        for (Machine * const machine : output_product->machines_needed) {
            input_output_matrix(machine->id, output_product->id) =
                machine_use_hours / machine->lifetime;
        }
        labor_vector(output_product->id) =
            output_product->living_labor_per_unit;
    }
}

double get_max_eigenvalue(Eigen::MatrixXd &io_matrix) {
    Eigen::EigenSolver<Eigen::MatrixXd> eigen_solver(io_matrix, false);
    Eigen::VectorXcd eigenvalues = eigen_solver.eigenvalues();
    double max_eigenvalue = 0.0;
    for (size_t i = 0; i < static_cast<unsigned long>(eigenvalues.size()); ++i) {
        if (eigenvalues(i).real() > max_eigenvalue &&
            !eigenvalues(i).imag()) {
            max_eigenvalue = eigenvalues(i).real();
        }
    }
    return max_eigenvalue;
}

std::vector<Producer *> &Society::get_producers() {
    return producers;
}

double Society::get_busyness() {
    double busyness = 0.0;
    for (Person * person : people) {
        busyness += person->get_busyness();
    }
    return busyness / people.size();
}

double Society::get_total_employment() {
    unsigned int employed = 0;
    for (Person * person : people) {
        employed += (person->get_firm() != nullptr);
    }
    return static_cast<double>(employed) / people.size();
}

void Society::log_total_employment() {
    Logger::get_instance()->log(Logger::SOCIETY, "employment", id, get_total_employment());
}

void Society::adjust_io_matrix(
    Eigen::MatrixXd& io_matrix,
    double max_eigenvalue) {
    io_matrix /= (max_eigenvalue + PRODUCT_INPUT_EPSILON);
    const size_t dim = io_matrix.rows();
    for (std::size_t j = 0; j < dim; ++j) {
        for (std::pair<Good * const, double>& input : products[j]->inputs_per_unit) {
            input.second = io_matrix(input.first->id, j);
        }
    }
}

Eigen::MatrixXd get_leontief_inverse(
    Eigen::MatrixXd io_matrix) {
    const std::size_t dim = io_matrix.rows();
    Eigen::MatrixXd identity_matrix = Eigen::MatrixXd::Identity(dim, dim);
    Eigen::MatrixXd leontief_matrix = identity_matrix - io_matrix;
    return leontief_matrix.inverse();
}

void Society::set_product_prices_production_consumption() {
    const size_t dim = products.size();
    Eigen::MatrixXd A(dim, dim);
    Eigen::VectorXd l(dim);
    populate_io_matrix_and_labor_vector(A, l);
    double max_eigenvalue = get_max_eigenvalue(A);
    if (max_eigenvalue >= 1.0) {
        adjust_io_matrix(A, max_eigenvalue);
    }
    log_io_matrix(A, dim);
    log_labor_vector(l, dim);
    Eigen::MatrixXd leontief_inverse = get_leontief_inverse(A);
    Eigen::VectorXd values = leontief_inverse.transpose() * l;
    for (std::size_t i = 0; i < dim; ++i) {
        if (values(i) <= 0.0) {
            std::stringstream message;
            message << "Value of item " << i << " <= 0.";
            throw std::domain_error(message.str());
        }
        products[i]->price_per_unit = values(i);
    }
    double consumption_scalar = 0.0;
    for (ConsumerGood * consumer_good : consumer_goods) {
        consumption_scalar += consumer_good->price_per_unit *
            consumer_good->mean_consumption_frequency;
    }
    const unsigned int initial_work_week =
        Sim::get_work_hours_daily() * Sim::get_work_days_weekly();
    consumption_scalar = PRODUCT_CONSUMPTION_MULT
        * initial_work_week
        / WEEK
        / consumption_scalar;
    for (ConsumerGood * consumer_good : consumer_goods) {
        consumer_good->mean_consumption_frequency *= consumption_scalar;
    }
    Eigen::VectorXd demands(dim);
    for (ConsumerGood * consumer_good : consumer_goods) {
        demands[consumer_good->id] = consumer_good->mean_consumption_frequency;
    }
    Eigen::VectorXd production = leontief_inverse * demands;
    for (std::size_t i = 0; i < dim; ++i) {
        initial_production[products[i]] = production(i);
    }
}

std::vector<Product *>& Society::get_products() {
    return products;
}

std::vector<Good *>& Society::get_goods() {
    return goods;
}

std::vector<ConsumerGood *>& Society::get_consumer_goods() {
    return consumer_goods;
}

std::vector<Machine *>& Society::get_machines() {
    return machines;
}

double Society::get_underlying_living_labor_per_unit(Product * product) {
    return underlying_living_labor_per_unit[product];
}

void Society::set_underlying_living_labor_per_unit(Product * product, double ll) {
    underlying_living_labor_per_unit[product] = ll;
}

std::vector<Distributor *> &Society::get_distributors() {
    return distributors;
}

std::unordered_set<Person *>& Society::get_unemployed_people() {
    return unemployed_people;
}

unsigned int Society::get_current_work_hours_daily() {
    return current_work_hours_daily;
}

unsigned int Society::get_current_work_days_weekly() {
	return current_work_days_weekly;
}

void Society::set_initial_account() {
    initial_account = 0.0;
    for (ConsumerGood * consumer_good : consumer_goods) {
        initial_account += consumer_good->price_per_unit *
                           consumer_good->mean_consumption_frequency;
    }
    initial_account *= INITIAL_ACCOUNT_DURATION;
}

int Society::get_initial_account() {
    return initial_account;
}

std::unordered_map<Product *, double> &Society::get_initial_production() {
    return initial_production;
}

void Society::update_work_hours_daily() {
    current_work_hours_daily = std::ceil(get_busyness() * INEFFICIENCY_OF_WORK * 
            WEEK / Sim::get_work_days_weekly());
    current_work_hours_daily = std::min(DAY, current_work_hours_daily);
}

Person * Society::birth_person() {
    Person * person = new Person(this);
    people.push_back(person);
    unemployed_people.insert(person);
    return person;
}

void Society::retire_person(Person *person) {
    // unimplemented until hiring/reallocation is done
}

void Society::log_io_matrix(Eigen::MatrixXd& A, size_t dim) {
    Logger::get_instance()->log(Logger::SOCIETY, "A_dim", id, static_cast<int>(dim));
    for (size_t i = 0; i < dim; ++i) {
        for (size_t j = 0; j < dim; ++j) {
            if (A(i, j)) {
                Logger::get_instance()->log(
                        Logger::SOCIETY,
                        "A",
                        id,
                        std::make_pair(i, j),
                        A(i, j)
                        );
            }
        }
    }
}

void Society::log_labor_vector(Eigen::VectorXd& l, size_t dim) {
    Logger::get_instance()->log(Logger::SOCIETY, "l_dim", id, static_cast<int>(dim));
    for (size_t i = 0; i < dim; ++i) {
        if (l(i)) {
            Logger::get_instance()->log(Logger::SOCIETY, "l", id, i, l(i));
        }
    }
}

void Society::log_consumption_frequencies() {
    Logger * logger = Logger::get_instance();
    for (const ConsumerGood * consumer_good : consumer_goods) {
        logger->log(
                Logger::SOCIETY,
                "mean_consumption_frequency",
                id,
                "",
                consumer_good->mean_consumption_frequency
                );
    }
}
