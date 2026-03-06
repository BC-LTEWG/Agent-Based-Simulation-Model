#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "ConsumerGood.h"
#include "Distributor.h"
#include "Firm.h"
#include "Logger.h"
#include "Machine.h"
#include "Person.h"
#include "Product.h"
#include "Producer.h"
#include "Sim.h"
#include "Society.h"

Society * Society::get_instance() {
    static Society * instance = new Society;
    return instance;
}

Society::Society() {
    static unsigned int unique_id = 0;
    id = unique_id++;
    set_initial_products();
    for (Product * product : products) {
        Logger::get_instance()->log(Logger::SOCIETY, "price", product->id, product->price_per_unit); 
        Logger::get_instance()->log(Logger::SOCIETY, "order_size", product->id, product->order_size);
    }
    for (int i = 0; i < STARTING_NUM_PRODUCERS; i++) {
        Producer * producer = new Producer(this, {goods[i %
                STARTING_NUM_PRODUCTS]});
        producers.push_back(producer);
        firms.push_back(producer);
    }
    for (int i = 0; i < STARTING_NUM_DISTRIBUTORS; i++) {
        Distributor * distributor =
            new Distributor(this, {goods[i % STARTING_NUM_PRODUCTS]});
        distributors.push_back(distributor);
        firms.push_back(distributor);
    }
    for (Firm * firm : firms) {
        for (Producer * producer : producers) {
            if (producer != firm) {
                firm->add_supplier(producer);
            }
        }
    }
    for (int i = 0; i < STARTING_NUM_PEOPLE; i++) {
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
}

void Society::set_initial_products() {
    std::size_t i = 0;
    for (; i < STARTING_NUM_PRODUCTS; ++i) {
        Product * new_product = new Product("Product_" + std::to_string(i));
        new_product->id = i;
        goods.push_back(new_product);
        products.push_back(new_product);
        product_to_index[new_product] = i;
    }
    static std::uniform_int_distribution<>
        machine_lifetime_dist(MACHINE_LIFETIME_MIN, MACHINE_LIFETIME_MAX);
    for (std::size_t j = 0; j < STARTING_NUM_MACHINES; ++j, ++i) {
        Machine * new_machine = new Machine(
                "Machine " + std::to_string(i),
                machine_lifetime_dist(Sim::get_random_generator())
                );
        machines.push_back(new_machine);
        products.push_back(new_machine);
        product_to_index[new_machine] = i;
    }
    for (Product * product: products) {
        product->set_inputs(goods);
        product->set_machines(machines);
    }
    set_product_prices();
}

void Society::populate_io_matrix_and_labor_vector(
        std::unordered_map<Product *, std::size_t>& product_to_index,
        Eigen::MatrixXd& input_output_matrix,
        Eigen::VectorXd& labor_vector
        ) {
    static const int starting_num_firms = STARTING_NUM_PRODUCERS +
        STARTING_NUM_DISTRIBUTORS;
    static const int average_team_size =
        STARTING_NUM_PEOPLE / starting_num_firms;
    for (Product * output_product : products) {
        for (const std::pair<Product * const, double>& input :
                output_product->inputs_per_unit) {
            input_output_matrix(
                    product_to_index[input.first],
                    product_to_index[output_product]
                    ) = input.second;
        }
        double machine_use_hours =
            output_product->living_labor_per_unit / average_team_size;
        for (Machine * const machine : output_product->machines_needed) {
            input_output_matrix(
                    product_to_index[static_cast<Product * const>(machine)],
                    product_to_index[output_product]
                    ) = machine_use_hours / machine->lifetime;
        }
        labor_vector(product_to_index[output_product]) =
            output_product->living_labor_per_unit;
    }
}

double get_max_eigenvalue(Eigen::MatrixXd& io_matrix) {
    Eigen::EigenSolver<Eigen::MatrixXd> eigen_solver(io_matrix, false);
    Eigen::VectorXcd eigenvalues = eigen_solver.eigenvalues();
    double max_eigenvalue = 0.0;
    for (size_t i = 0; i < eigenvalues.size(); ++i) {
        if (eigenvalues(i).real() > max_eigenvalue &&
                !eigenvalues(i).imag()) {
            max_eigenvalue = eigenvalues(i).real();
        }
    }
    return max_eigenvalue;
}

std::vector<Producer *>& Society::get_producers() {
    return producers;
}

void Society::adjust_io_matrix(
        Eigen::MatrixXd& io_matrix,
        double max_eigenvalue
        ) {
    io_matrix /= (max_eigenvalue + PRODUCT_INPUT_EPSILON);
    const size_t dim = io_matrix.rows();
    for (std::size_t j = 0; j < dim; ++j) {
        for (std::size_t i = 0; i < dim; ++i) {
            if (io_matrix(i, j)) {
                products[j]->inputs_per_unit[products[i]] = io_matrix(i, j);
            }
        }
    }
}

Eigen::VectorXd get_leontief_function(
        Eigen::MatrixXd io_matrix, 
        Eigen::VectorXd labor
        ) {
    Eigen::MatrixXd io_matrix_transpose = io_matrix.transpose();
    const std::size_t dim = io_matrix.rows();
    Eigen::MatrixXd identity_matrix = Eigen::MatrixXd::Identity(dim, dim);
    Eigen::MatrixXd leontief_matrix = identity_matrix - io_matrix_transpose;
    Eigen::MatrixXd leontief_matrix_inverse = leontief_matrix.inverse();
    return leontief_matrix_inverse * labor;
}

void Society::set_product_prices() {
    const size_t dim = products.size();
    Eigen::MatrixXd A(dim, dim);
    Eigen::VectorXd l(dim);
    populate_io_matrix_and_labor_vector(product_to_index, A, l);
    double max_eigenvalue = get_max_eigenvalue(A);
    if (max_eigenvalue >= 1.0) {
        adjust_io_matrix(A, max_eigenvalue);
    }
    Eigen::VectorXd values = get_leontief_function(A, l);
    for (std::size_t i = 0; i < dim; ++i) {
        if (values(i) <= 0.0) {
            std::stringstream message;
            message << "Value of item " << i << " <= 0.";
            throw std::domain_error(message.str());
        }
        products[i]->price_per_unit = values(i);
    }
}

std::vector<Product *>& Society::get_products() {
    return products;
}

std::vector<Product *>& Society::get_goods() {
    return goods;
}

ConsumerGood * Society::get_consumer_good(Product * product) {
    if (consumer_goods.count(product)) {
        return consumer_goods[product];
    } else {
        return NULL;
    }
}

void Society::add_consumer_good(Product * product) {
    if (!consumer_goods.count(product)) {
        consumer_goods[product] = new ConsumerGood(product);
    }
}

std::vector<Distributor *>& Society::get_distributors() {
    return distributors;
}

std::vector<Person *>& Society::get_unemployed_people() {
    return unemployed_people;
}

unsigned int Society::get_current_work_hours_daily() {
    return current_work_hours_daily;
}

int Society::get_current_work_days_weekly() {
	return current_work_days_weekly;
}

void Society::set_initial_account() {
    initial_account = 0.0;
    for (Product * product : products) {
        ConsumerGood * consumer_good = get_consumer_good(product);
        if(!consumer_good) {
            std::cerr << "consumer good DNE" << std::endl;
        }

        initial_account += consumer_good->price_per_unit *
            consumer_good->mean_consumption_frequency *
            PERSON_SHOPPING_PERIOD; 
    }
}

int Society::get_initial_account() {
    return initial_account;
}

Person * Society::birth_person() {
    Person * person = new Person(this);
    people.push_back(person);
    unemployed_people.push_back(person);
    return person;
}

void Society::retire_person(Person * person) {
    // unimplemented until hiring/reallocation is done
}

