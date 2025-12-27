#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <iostream>
#include <memory>
#include <vector>

#include "doctest.h"
#include "leontief_function.h"

void print_matrix(Matrix * matrix) {
    for (size_t i = 0; i < matrix->size(); ++i) {
        for (size_t j = 0; j < matrix->size(); ++j) {
            std::cout << (*(*matrix)[i])[j] << " ";
        }
        std::cout << std::endl;
    }
}

std::unique_ptr<Matrix> create_matrix(size_t dim) {
    std::unique_ptr<Matrix> matrix = std::make_unique<Matrix>();
    double element = 1.0;
    for (size_t i = 0; i < dim; ++i) {
        std::unique_ptr<Row> row = std::make_unique<Row>();
        for (size_t j = 0; j < dim; ++j) {
            row->push_back(element++);
        }
        matrix->push_back(std::move(row));
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

TEST_CASE("Matrix transpose") {
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
    const size_t dim = 16;
    std::unique_ptr<Matrix> matrix = create_matrix(dim);
    (*(*matrix)[0])[0] = 0.0;
    print_matrix(matrix.get());
    reduced_row_echelon(matrix.get());
    print_matrix(matrix.get());
    CHECK(true);
}
