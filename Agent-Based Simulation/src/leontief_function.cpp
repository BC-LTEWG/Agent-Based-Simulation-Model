#include <iostream>
#include <memory>
#include <vector>

#include "leontief_function.h"

void print_m(Matrix * matrix) {
    for (size_t row = 0; row < matrix->size(); ++row) {
        for (size_t col = 0; col < (*matrix)[row]->size(); ++col) {
            std::cout << (*(*matrix)[row])[col] << " ";
        }
        std::cout << std::endl;
    }
}

void transpose(Matrix * matrix_a) {
    for (size_t i = 0; i < matrix_a->size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            double tmp = (*(*matrix_a)[i])[j];
            (*(*matrix_a)[i])[j] = (*(*matrix_a)[j])[i];
            (*(*matrix_a)[j])[i] = tmp;
        }
    }
}

void augment(Matrix * matrix) {
    size_t dim = matrix->size();
    for (size_t row = 0; row < dim; ++row) {
        for (size_t col = 0; col < dim; ++col) {
            if (row == col) {
                (*matrix)[row]->push_back(1.0);
            } else {
                (*matrix)[row]->push_back(0.0);
            }
        }
    }
}
void hoist_maximum_pivot_row(Matrix * matrix, size_t rows, size_t pivot_row) {
    size_t pivot_col = pivot_row;
    double pivot_max = (*(*matrix)[pivot_col])[pivot_row];
    size_t max_row = pivot_row;
    for (size_t row = pivot_row + 1; row < rows; ++row) {
        if ((*(*matrix)[row])[pivot_col] > pivot_max) {
            pivot_max = (*(*matrix)[row])[pivot_col];
            max_row = row;
        }
    }
    if (max_row != pivot_row) {
        std::swap((*matrix)[max_row], (*matrix)[pivot_row]);
    }
}

void round_to_zero(Matrix * matrix, size_t rows, size_t cols) {
    const double tolerance = 1.0e-14;
    for (size_t row = 0; row < rows; ++row) {
        for (size_t col = 0; col < cols; ++col) {
            if ((*(*matrix)[row])[col] < tolerance) {
                (*(*matrix)[row])[col] = 0.0;
            }
        }
    }
}

void form_echelon_row_and_reduce(
        Matrix * matrix,
        size_t rows,
        size_t cols,
        size_t pivot_row
        ) {
    size_t pivot_col = pivot_row;
    double pivot = (*(*matrix)[pivot_row])[pivot_col];
    if (!pivot) {
        std::cerr << "Matrix is not invertible." << std::endl;
        return;
    }
    for (size_t row = pivot_row + 1; row < rows; ++row) {
        double pivot_row_multiplier = (*(*matrix)[row])[pivot_col] / pivot;
        for (size_t col = pivot_col; col < cols; ++col) {
            (*(*matrix)[row])[col] -= pivot_row_multiplier * (*(*matrix)[pivot_row])[col];
        }
    }
    double reciprocal = 1 / pivot;
    for (size_t col = pivot_col; col < cols; ++col) {
        (*(*matrix)[pivot_row])[col] *= reciprocal;
    }
}

void reduced_row_echelon_form(Matrix * matrix, size_t rows, size_t cols) {
    for (size_t pivot_row = 0; pivot_row < rows; ++pivot_row) {
        hoist_maximum_pivot_row(matrix, rows, pivot_row);
        form_echelon_row_and_reduce(matrix, rows, cols, pivot_row);
    }
}

std::unique_ptr<Matrix> inverse(Matrix * matrix) {
    size_t dim = matrix->size();
    augment(matrix);
    std::cout << "Augmented" << std::endl;
    print_m(matrix);
    std::cout << std::endl;
    reduced_row_echelon_form(matrix, dim, dim * 2);
    std::cout << "RREF" << std::endl;
    print_m(matrix);
    std::cout << std::endl;
    std::unique_ptr<Matrix> matrix_inverse = std::make_unique<Matrix>();
    for (size_t row = 0; row < dim; ++row) {
        std::unique_ptr<Row> new_row = std::make_unique<Row>();
        for (size_t col = dim; col < 2 * dim; ++col) {
            new_row->push_back((*(*matrix)[row])[col]);
        }
        matrix_inverse->push_back(std::move(new_row));
    }
    std::cout << "Inverse" << std::endl;
    print_m(matrix_inverse.get());
    std::cout << std::endl;

    return matrix_inverse;
}
