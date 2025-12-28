#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "doctest.h"
#include "leontief_function.h"
#include "Sim.h"

TEST_SUITE_BEGIN("Leontief Function");

void print_matrix(Matrix * matrix) {
    for (size_t row = 0; row < matrix->size(); ++row) {
        for (size_t col = 0; col < (*matrix)[row]->size(); ++col) {
            std::cout << (*(*matrix)[row])[col] << " ";
        }
        std::cout << std::endl;
    }
}

std::unique_ptr<Matrix> multiply_matrices(Matrix * a, Matrix * b) {
    std::unique_ptr<Matrix> c = std::make_unique<Matrix>();
    for (size_t i = 0; i < a->size(); ++i) {
        std::unique_ptr<Row> new_row = std::make_unique<Row>();
        for (size_t j = 0; j < b->size(); ++j) {
            double element = 0.0;
            for (size_t k = 0; k < a->size(); ++k) {
                element += ((*(*a)[i])[k] * (*(*b)[k])[j]);
            }
            new_row->push_back(element);
        }
        c->push_back(std::move(new_row));
    }
    return c;
}

std::unique_ptr<Matrix> create_matrix(size_t dim) {
    std::unique_ptr<Matrix> matrix = std::make_unique<Matrix>();
    const unsigned int seed = 123;
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dist(0.0, 0.9);
    for (size_t i = 0; i < dim; ++i) {
        std::unique_ptr<Row> row = std::make_unique<Row>();
        for (size_t j = 0; j < dim; ++j) {
            row->push_back(dist(gen));
        }
        matrix->push_back(std::move(row));
    }
    for (size_t row = 0; row < dim; ++row) {
        (*(*matrix)[row])[row] = 1 - (*(*matrix)[row])[row];
    }
    return matrix;
}

std::unique_ptr<Matrix> copy_matrix(Matrix * orig) {
    std::unique_ptr<Matrix> copy = std::make_unique<Matrix>();
    for (size_t i = 0; i < orig->size(); ++i) {
        std::unique_ptr<Row> row = std::make_unique<Row>();
        for (size_t j = 0; j < (*orig)[i]->size(); ++j) {
            row->push_back((*(*orig)[i])[j]);
        }
        copy->push_back(std::move(row));
    }
    return copy;
}

bool is_upper_triangular(Matrix * matrix) {
    size_t row = 0, col = 0;
    bool done = false;
    for (; row < matrix->size(); ++row) {
        for (; col < matrix->size(); ++col) {
            if ((*(*matrix)[row])[col]) {
                done = true;
                break;
            }
        }
        if (done) {
            break;
        }
    }
    if (row == matrix->size()) {
        std::cout << "Matrix is all 0s." << std::endl;
        return false;
    }
    ++row;
    for (; row < matrix->size(); ++row) {
        for (size_t col = 0; col < row; ++col) {
            if ((*(*matrix)[row])[col]) {
                return false;
            }
        }
    }
    return true;
}

bool is_reduced(Matrix * matrix) {
    for (size_t row = 0; row < matrix->size(); ++row) {
        size_t col = 0;
        while (!(*(*matrix)[row])[col]) {
            ++col;
        }
        if ((*(*matrix)[row])[col] != 1.0) {
            return false;
        }
    }
    return true;
}

bool is_identity_matrix(Matrix * matrix, size_t dim, size_t first_col) {
    for (size_t row = 0; row < dim; ++row) {
        for (size_t col = 0; col < dim; ++col) {
            double element = (*(*matrix)[row])[col + first_col];
            if (col == row) {
               if (element != 1.0) {
                   return false;
               }
            } else {
                if (element) {
                    return false;
                }
            }
        }
    }
    return true;
}

TEST_CASE("Transpose matrix") {
    const size_t dim = 16;
    std::unique_ptr<Matrix> trnspose = create_matrix(dim);
    std::unique_ptr<Matrix> orig = copy_matrix(trnspose.get());
    transpose(trnspose.get());
    CHECK(orig->size() == trnspose->size());
    CHECK((*orig)[0]->size() == orig->size());
    for (size_t i = 0; i < orig->size(); ++i) {
        CHECK((*orig)[i]->size() == (*trnspose)[i]->size());
    }
    for (size_t i = 0; i < orig->size(); ++i) {
        for (size_t j = 0; j < orig->size(); ++j) {
            CHECK((*(*orig)[i])[j] == (*(*trnspose)[j])[i]);
        }
    }
}

TEST_CASE("Reduced row echelon form") {
    const size_t dim = 8;
    std::unique_ptr<Matrix> matrix = create_matrix(dim);
    reduced_row_echelon_form(matrix.get(), matrix->size(), matrix->size());
    std::cout << std::endl;
    CHECK(is_upper_triangular);
    CHECK(is_reduced);
}

TEST_CASE("Augment matrix") {
    const size_t dim = 8;
    std::unique_ptr<Matrix> matrix = create_matrix(dim);
    augment(matrix.get());
    CHECK(is_identity_matrix(matrix.get(), dim, dim));
}

TEST_CASE("Inverse matrix") {
    const size_t dim = 8;
    std::unique_ptr<Matrix> matrix = create_matrix(dim);
    std::unique_ptr<Matrix> orig = copy_matrix(matrix.get());
    std::unique_ptr<Matrix> matrix_inverse = inverse(matrix.get());
    print_matrix(matrix_inverse.get());
    std::cout << std::endl;
    std::unique_ptr<Matrix> product = multiply_matrices(orig.get(), matrix_inverse.get());
    round_to_zero(product.get(), product->size(), product->size());
    print_matrix(product.get());
}

TEST_SUITE_END();

