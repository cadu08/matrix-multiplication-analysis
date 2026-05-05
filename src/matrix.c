#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

void fill_random_matrix(int *matrix, int n) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = (rand() % 9) + 1;
    }
}

void print_matrix(int *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", matrix[i * n + j]);
        }
        printf("\n");
    }
}

void write_matrix_to_file(const char *filename, int *matrix, int n) {
    FILE *f = fopen(filename, "w");

    if (f == NULL) {
        printf("Error opening file: %s\n", filename);
        return;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            fprintf(f, "%d ", matrix[i * n + j]);
        }
        fprintf(f, "\n");
    }

    fclose(f);
}