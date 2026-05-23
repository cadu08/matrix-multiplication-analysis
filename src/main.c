#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "matrix.h"
#include "iterative.h"
#include "divide_conquer.h"
#include "hybrid_divide_conquer.h"
#include "hybrid_strassen.h"
#include "strassen.h"
#include "heap_tracker.h"

#define NUM_SIZES 5
#define NUM_PAIRS 10
#define FIXED_SEED 42
#define HYBRID_DIVIDE_CONQUER_THRESHOLD 64
#define HYBRID_STRASSEN_THRESHOLD 64

typedef void (*matrix_multiply_fn)(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]);

void run_experiment(
    FILE *file,
    const char *algorithm_name,
    matrix_multiply_fn multiply,
    int n,
    int pair_id,
    matrix_value_t *A_data,
    matrix_value_t *B_data
) {
    matrix_value_t (*A)[n] = (matrix_value_t (*)[n]) A_data;
    matrix_value_t (*B)[n] = (matrix_value_t (*)[n]) B_data;

    matrix_value_t *C_data = malloc(n * n * sizeof(matrix_value_t));
    if (C_data == NULL) {
        fprintf(stderr, "Error allocating result matrix C\n");
        exit(1);
    }

    matrix_value_t (*C)[n] = (matrix_value_t (*)[n]) C_data;

    long memory_before = get_memory_usage_kb();
    heap_tracker_reset();
    double start = get_time();

    multiply(n, A, B, C);

    double end = get_time();
    heap_stats_t heap_stats = heap_tracker_get_stats();
    long memory_after = get_memory_usage_kb();

    fprintf(
        file,
        "%s,%d,%d,%.9f,%ld,%ld,%ld,%zu,%zu,%zu\n",
        algorithm_name,
        n,
        pair_id,
        end - start,
        memory_before,
        memory_after,
        memory_after - memory_before,
        heap_stats.current_bytes,
        heap_stats.peak_bytes,
        heap_stats.allocation_count
    );

    free(C_data);
}

void multiply_hybrid_divide_conquer_wrapper(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]) {
    multiply_hybrid_divide_conquer(n, A, B, C, HYBRID_DIVIDE_CONQUER_THRESHOLD);
}

void multiply_hybrid_strassen_wrapper(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]) {
    multiply_hybrid_strassen(n, A, B, C, HYBRID_STRASSEN_THRESHOLD);
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
        "algorithm,matrix_size,pair_id,time_seconds,memory_before_kb,memory_after_kb,memory_difference_kb,heap_current_bytes,heap_peak_bytes,heap_allocations\n"
    );

    srand(FIXED_SEED);

    for (int s = 0; s < NUM_SIZES; s++) {
        int n = sizes[s];

        printf("Running experiments for size %d x %d...\n", n, n);

        for (int pair_id = 1; pair_id <= NUM_PAIRS; pair_id++) {
            matrix_value_t *A = malloc(n * n * sizeof(matrix_value_t));
            matrix_value_t *B = malloc(n * n * sizeof(matrix_value_t));

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
            run_experiment(file, "hybrid_divide_conquer", multiply_hybrid_divide_conquer_wrapper, n, pair_id, A, B);
            run_experiment(file, "strassen", multiply_strassen, n, pair_id, A, B);
            run_experiment(file, "hybrid_strassen", multiply_hybrid_strassen_wrapper, n, pair_id, A, B);

            free(A);
            free(B);
        }
    }

    fclose(file);

    printf("Experiments completed.\n");
    printf("Results saved to results/experiment_results.csv\n");

    return 0;
}
