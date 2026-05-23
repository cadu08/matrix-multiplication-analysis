#include "iterative.h"

void multiply_iterative(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0;

            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}
