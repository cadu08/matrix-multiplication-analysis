#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "matrix.h"
#include "iterative.h"
#include "divide_conquer.h"
#include "hybrid.h"

#define NUM_SIZES 5
#define NUM_PAIRS 10
#define FIXED_SEED 42
#define HYBRID_THRESHOLD 64

typedef void (*matrix_multiply_fn)(int n, int A[n][n], int B[n][n], int C[n][n]);

void run_experiment(
    FILE *file,
    const char *algorithm_name,
    matrix_multiply_fn multiply,
    int n,
    int pair_id,
    int *A_data,
    int *B_data
) {
    int (*A)[n] = (int (*)[n]) A_data;
    int (*B)[n] = (int (*)[n]) B_data;

    int *C_data = malloc(n * n * sizeof(int));
    if (C_data == NULL) {
        fprintf(stderr, "Error allocating result matrix C\n");
        exit(1);
    }

    int (*C)[n] = (int (*)[n]) C_data;

    long memory_before = get_memory_usage_kb();
    double start = get_time();

    multiply(n, A, B, C);

    double end = get_time();
    long memory_after = get_memory_usage_kb();

    fprintf(
        file,
        "%s,%d,%d,%.9f,%ld,%ld,%ld\n",
        algorithm_name,
        n,
        pair_id,
        end - start,
        memory_before,
        memory_after,
        memory_after - memory_before
    );

    free(C_data);
}

void multiply_hybrid_wrapper(int n, int A[n][n], int B[n][n], int C[n][n]) {
    multiply_hybrid(n, A, B, C, HYBRID_THRESHOLD);
}

int main() {
    int sizes[NUM_SIZES] = {64, 128, 256, 512, 1024};

    FILE *file = fopen("results/experiment_results.csv", "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening results file\n");
        return 1;
    }

    fprintf(
        file,
        "algorithm,matrix_size,pair_id,time_seconds,memory_before_kb,memory_after_kb,memory_difference_kb\n"
    );

    srand(FIXED_SEED);

    for (int s = 0; s < NUM_SIZES; s++) {
        int n = sizes[s];

        printf("Running experiments for size %d x %d...\n", n, n);

        for (int pair_id = 1; pair_id <= NUM_PAIRS; pair_id++) {
            int *A = malloc(n * n * sizeof(int));
            int *B = malloc(n * n * sizeof(int));

            if (A == NULL || B == NULL) {
                fprintf(stderr, "Error allocating input matrices\n");
                fclose(file);
                free(A);
                free(B);
                return 1;
            }

            fill_random_matrix(A, n);
            fill_random_matrix(B, n);

            run_experiment(file, "iterative", multiply_iterative, n, pair_id, A, B);
            run_experiment(file, "recursive", multiply_divide_conquer, n, pair_id, A, B);
            run_experiment(file, "hybrid", multiply_hybrid_wrapper, n, pair_id, A, B);

            free(A);
            free(B);
        }
    }

    fclose(file);

    printf("Experiments completed.\n");
    printf("Results saved to results/experiment_results.csv\n");

    return 0;
}