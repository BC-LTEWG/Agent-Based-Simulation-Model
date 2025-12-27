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

TEST_CASE("Matrix transpose") {
    const size_t dim = 16;
    std::unique_ptr<Matrix> matrix = std::make_unique<Matrix>();
    std::unique_ptr<Matrix> copy = std::make_unique<Matrix>();
    double element = 1.0;
    for (size_t i = 0; i < dim; ++i) {
        std::unique_ptr<Row> row = std::make_unique<Row>();
        std::unique_ptr<Row> row_copy = std::make_unique<Row>();
        for (size_t j = 0; j < dim; ++j) {
            row->push_back(element);
            row_copy->push_back(element++);
        }
        matrix->push_back(std::move(row));
        copy->push_back(std::move(row_copy));
    }
    transpose(matrix.get());
    CHECK(copy->size() == matrix->size());
    CHECK((*copy)[0]->size() == copy->size());
    for (size_t i = 0; i < copy->size(); ++i) {
        CHECK((*copy)[i]->size() == (*matrix)[i]->size());
    }
    for (size_t i = 0; i < copy->size(); ++i) {
        for (size_t j = 0; j < copy->size(); ++j) {
            CHECK((*(*copy)[i])[j] == (*(*matrix)[j])[i]);
        }
    }
}
