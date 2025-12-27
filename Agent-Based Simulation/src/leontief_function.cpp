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
            for (size_t j = i; j < matrix->size(); ++j) {
                if ((*(*matrix)[i])[j]) {
                    std::swap(matrix[i], matrix[j]);
                }
            }
        }
        if ((*(*matrix)[i])[i]) {
        }
    }
}
