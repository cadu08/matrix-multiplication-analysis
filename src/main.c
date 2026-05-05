#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "matrix.h"

#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

int main() {
    srand(RANDOM_SEED);

    for (int s = 0; s < NUM_SIZES; s++) {
        int n = MATRIX_SIZES[s];

        printf("Matrix size: %d x %d\n", n, n);

        for (int run = 0; run < NUM_RUNS; run++) {
            int *A = malloc(n * n * sizeof(int));
            int *B = malloc(n * n * sizeof(int));

            if (A == NULL || B == NULL) {
                printf("Memory allocation failed.\n");
                free(A);
                free(B);
                return 1;
            }

            fill_random_matrix(A, n);
            fill_random_matrix(B, n);

            printf("  Generated pair %d/%d\n", run + 1, NUM_RUNS);

            free(A);
            free(B);
        }
    }

    return 0;
}