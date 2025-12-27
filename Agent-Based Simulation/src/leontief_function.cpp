#include <iostream>
#include <vector>

#include "leontief_function.h"

void transpose(Matrix * matrix_a) {
    for (size_t i = 0; i < matrix_a->size(); ++i) {
        for (size_t j = 0; j < i; ++j) {
            double tmp = (*(*matrix_a)[i])[j];
            (*(*matrix_a)[i])[j] = (*(*matrix_a)[j])[i];
            (*(*matrix_a)[j])[i] = tmp;
        }
    }
}

void reduced_row_echelon(Matrix * matrix) {
    for (size_t i = 0; i < matrix->size() - 1; ++i) {
        if (!(*(*matrix)[i])[i]) {
            size_t j = i;
            for (size_t k = j; k < matrix->size(); ++k) {
                if ((*(*matrix)[k])[j]) {
                    std::swap((*matrix)[k], (*matrix)[i]);
                    break;
                }
            }
        }
        double pivot = (*(*matrix)[i])[i];
        if (pivot) {
            for (size_t k = i + 1; k < matrix->size(); ++k) {
                double pivot_row_multiplier = (*(*matrix)[k])[i] / pivot;
                for (size_t j = i; j < matrix->size(); ++j) {
                    (*(*matrix)[k])[j] -= pivot_row_multiplier * (*(*matrix)[i])[j];
                }
            }
        }
    }
}
