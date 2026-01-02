#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <unordered_map>

#include "Distributor.h"
#include "Firm.h"
#include "Person.h"
#include "Product.h"
#include "Producer.h"
#include "Society.h"

Society * Society::instance = nullptr;

Society::Society() {
    instance = this;
    set_initial_products();
    // note: no way to assign products to producers or suppliers
    // to distributors yet
    for (int i = 0; i < STARTING_NUM_PRODUCERS; i++) {
        Producer * producer = new Producer({products[i %
                STARTING_NUM_PRODUCTS]});
        producers.push_back(producer);
        firms.push_back(producer);
    }
    for (int i = 0; i < STARTING_NUM_DISTRIBUTORS; i++) {
        Distributor * distributor =
            new Distributor({products[i % STARTING_NUM_PRODUCTS]});
        distributors.push_back(distributor);
        firms.push_back(distributor);
    }
    // add suppliers to firms
    for (Firm * firm : firms) {
        for (Producer * producer : producers) {
            if (producer != firm) {
                firm->add_supplier(producer);
            }
        }
    }
    // people MUST come after products and distributors are created
    for (int i = 0; i < STARTING_NUM_PEOPLE; i++) {
        birth_person();	
    }
}

void Society::set_initial_products() {
    for (std::size_t i = 0; i < STARTING_NUM_PRODUCTS; ++i) {
        products.push_back(new Product("Product " + std::to_string(i)));
    }
    static std::uniform_int_distribution<>
        machine_lifetime_dist(MACHINE_LIFETIME_MIN, MACHINE_LIFETIME_MAX);
    machine_lifetime = machine_lifetime_dist(Sim::gen);
    for (std::size_t i = 0; i < STARTING_NUM_MACHINES; ++i) {
        Machine * new_machine =
            new Machine("Machine " + std::to_string(i), machine_lifetime);
        machines.push_back(new_machine);
        products.push_back(new_machine);
    }
    for (Product * product: products) {
        product->set_inputs(products);
    }
    set_product_prices();
}

std::unordered_map<Product *, std::size_t>
Society::get_product_to_index_map() {
    std::unordered_map<Product *, size_t> product_to_index;
    for (size_t i = 0; i < products.size(); ++i) {
        product_to_index[products[i]] = i;
    }
    return product_to_index;
}

void Society::populate_io_matrix_and_labor_vector(
        std::unordered_map<Product *, std::size_t>& product_to_index,
        Eigen::MatrixXd& input_output_matrix,
        Eigen::VectorXd& labor_vector
        ) {
    for (Product * output_product : products) {
        for (const std::pair<Product * const, double>& input :
                output_product->inputs_per_unit) {
            input_output_matrix(
                    product_to_index[input.first],
                    product_to_index[output_product]
                    ) = input.second;
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
    std::unordered_map<Product *, size_t> product_to_index =
        get_product_to_index_map();
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
        if (values(i) < 0.0) {
            throw std::domain_error("Value < 0.");
        }
        products[i]->price_per_unit = values(i);
    }
}

/*
unsigned int get_estimated_average_worker_team_size() {
    unsigned int average_order_size = 
        (PRODUCT_ORDER_SIZE_MIN + PRODUCT_ORDER_SIZE_MAX + 1) / 2;
    unsigned int average_living_labor =
        (PRODUCT_LABOR_PER_UNIT_MIN + PRODUCT_LABOR_PER_UNIT_MIN + 1) / 2;
    return average_order_size;
}
*/

std::vector<Product *>& Society::get_products() {
    return products;
}

std::vector<Distributor *>& Society::get_distributors() {
    return distributors;
}

std::vector<Person *>& Society::get_unemployed_people() {
    return unemployed_people;
}

int Society::get_current_work_hours_daily() {
    return current_work_hours_daily;
}

int Society::get_current_work_days_weekly() {
	return current_work_days_weekly;
}

Person * Society::birth_person() {
    Person * person = new Person();
    people.push_back(person);
    unemployed_people.push_back(person);
    return person;
}

void Society::retire_person(Person * person) {
    // unimplemented until hiring/reallocation is done
}

