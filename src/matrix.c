#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

void fill_random_matrix(matrix_value_t *matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        /*
         * The +1 and +2 offsets map the finite rand() range to the open
         * interval (0, 1), avoiding special operands while preserving the
         * deterministic sequence controlled by srand().
         */
        matrix[i] = (rand() + 1.0) / (RAND_MAX + 2.0);
    }
}

void print_matrix(matrix_value_t *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%.17g ", matrix[i * n + j]);
        }
        printf("\n");
    }
}

void write_matrix_to_file(const char *filename, matrix_value_t *matrix, int n) {
    FILE *f = fopen(filename, "w");

    if (f == NULL) {
        printf("Error opening file: %s\n", filename);
        return;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(f, "%.17g ", matrix[i * n + j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}
