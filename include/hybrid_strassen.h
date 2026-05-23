#ifndef HYBRID_STRASSEN_H
#define HYBRID_STRASSEN_H

#include "matrix.h"

void multiply_hybrid_strassen(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n], int threshold);

#endif
