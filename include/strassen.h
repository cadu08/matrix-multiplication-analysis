#ifndef STRASSEN_H
#define STRASSEN_H

#include "matrix.h"

void multiply_strassen(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]);

#endif
