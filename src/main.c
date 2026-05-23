#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"
#include "matrix.h"
#include "iterative.h"
#include "divide_conquer.h"
#include "hybrid_divide_conquer.h"
#include "hybrid_strassen.h"
#include "strassen.h"
#include "heap_tracker.h"

#define DEFAULT_RESULTS_FILE "results/experiment_results.csv"

#ifndef NUM_SIZES
#define NUM_SIZES 5
#endif

#ifndef NUM_PAIRS
#define NUM_PAIRS 10
#endif

#ifndef FIXED_SEED
#define FIXED_SEED 42
#endif

#ifndef HYBRID_DIVIDE_CONQUER_THRESHOLD
#define HYBRID_DIVIDE_CONQUER_THRESHOLD 64
#endif

#ifndef HYBRID_STRASSEN_THRESHOLD
#define HYBRID_STRASSEN_THRESHOLD 64
#endif

#ifndef VALIDATION_ABS_TOLERANCE
#define VALIDATION_ABS_TOLERANCE 1e-8
#endif

#ifndef VALIDATION_REL_TOLERANCE
#define VALIDATION_REL_TOLERANCE 1e-9
#endif

typedef void (*matrix_multiply_fn)(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]);

typedef struct {
    int is_correct;
    matrix_value_t max_abs_error;
    matrix_value_t max_rel_error;
} validation_result_t;

static void ensure_results_directory_exists(void) {
    struct stat results_stat;

    if (mkdir("results", 0755) == -1) {
        if (errno != EEXIST) {
            perror("Error creating results directory");
            exit(EXIT_FAILURE);
        }

        if (stat("results", &results_stat) == -1 || !S_ISDIR(results_stat.st_mode)) {
            fprintf(stderr, "Error: results exists but is not a directory\n");
            exit(EXIT_FAILURE);
        }
    }
}

static validation_result_t validate_against_reference(
    int n,
    matrix_value_t candidate[n][n],
    matrix_value_t reference[n][n]
) {
    validation_result_t result = {1, 0.0, 0.0};

    /*
     * Floating-point matrix products can differ by operation order, especially
     * in Strassen's additions/subtractions. The absolute/relative envelope
     * checks numerical equivalence without charging validation work to timing.
     */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix_value_t abs_error = fabs(candidate[i][j] - reference[i][j]);
            matrix_value_t reference_magnitude = fabs(reference[i][j]);
            matrix_value_t rel_error = 0.0;

            if (reference_magnitude > 0.0) {
                rel_error = abs_error / reference_magnitude;
            } else if (abs_error > 0.0) {
                rel_error = INFINITY;
            }

            if (abs_error > result.max_abs_error) {
                result.max_abs_error = abs_error;
            }

            if (rel_error > result.max_rel_error) {
                result.max_rel_error = rel_error;
            }

            if (
                abs_error > VALIDATION_ABS_TOLERANCE &&
                rel_error > VALIDATION_REL_TOLERANCE
            ) {
                result.is_correct = 0;
            }
        }
    }

    return result;
}

void run_experiment(
    FILE *file,
    const char *algorithm_name,
    matrix_multiply_fn multiply,
    int n,
    int pair_id,
    matrix_value_t *A_data,
    matrix_value_t *B_data,
    matrix_value_t *reference_data
) {
    matrix_value_t (*A)[n] = (matrix_value_t (*)[n]) A_data;
    matrix_value_t (*B)[n] = (matrix_value_t (*)[n]) B_data;
    matrix_value_t (*reference)[n] = (matrix_value_t (*)[n]) reference_data;

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
    validation_result_t validation = validate_against_reference(n, C, reference);

    fprintf(
        file,
        "%s,%d,%d,%.9f,%ld,%ld,%ld,%zu,%zu,%zu,%d,%.17g,%.17g\n",
        algorithm_name,
        n,
        pair_id,
        end - start,
        memory_before,
        memory_after,
        memory_after - memory_before,
        heap_stats.current_bytes,
        heap_stats.peak_bytes,
        heap_stats.allocation_count,
        validation.is_correct,
        validation.max_abs_error,
        validation.max_rel_error
    );

    if (!validation.is_correct) {
        fprintf(
            stderr,
            "Validation failed for %s, size %d, pair %d: max_abs_error=%.17g, max_rel_error=%.17g\n",
            algorithm_name,
            n,
            pair_id,
            validation.max_abs_error,
            validation.max_rel_error
        );
        free(C_data);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    free(C_data);
}

void multiply_hybrid_divide_conquer_wrapper(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]) {
    multiply_hybrid_divide_conquer(n, A, B, C, HYBRID_DIVIDE_CONQUER_THRESHOLD);
}

void multiply_hybrid_strassen_wrapper(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]) {
    multiply_hybrid_strassen(n, A, B, C, HYBRID_STRASSEN_THRESHOLD);
}

int main(int argc, char *argv[]) {
    int sizes[] = {64, 128, 256, 512, 1024};
    int available_sizes = (int) (sizeof(sizes) / sizeof(sizes[0]));
    const char *results_file_path = DEFAULT_RESULTS_FILE;

    if (NUM_SIZES > available_sizes) {
        fprintf(stderr, "NUM_SIZES cannot exceed %d\n", available_sizes);
        return 1;
    }

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [results_csv_path]\n", argv[0]);
        return 1;
    }

    if (argc == 2) {
        results_file_path = argv[1];
    }

    ensure_results_directory_exists();

    FILE *file = fopen(results_file_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening results file: %s\n", results_file_path);
        return 1;
    }

    fprintf(
        file,
        "algorithm,matrix_size,pair_id,time_seconds,memory_before_kb,memory_after_kb,memory_difference_kb,heap_current_bytes,heap_peak_bytes,heap_allocations,is_correct,max_abs_error,max_rel_error\n"
    );

    srand(FIXED_SEED);

    for (int s = 0; s < NUM_SIZES; s++) {
        int n = sizes[s];

        printf("Running experiments for size %d x %d...\n", n, n);

        for (int pair_id = 1; pair_id <= NUM_PAIRS; pair_id++) {
            matrix_value_t *A = malloc(n * n * sizeof(matrix_value_t));
            matrix_value_t *B = malloc(n * n * sizeof(matrix_value_t));
            matrix_value_t *reference = malloc(n * n * sizeof(matrix_value_t));

            if (A == NULL || B == NULL || reference == NULL) {
                fprintf(stderr, "Error allocating input matrices\n");
                fclose(file);
                free(A);
                free(B);
                free(reference);
                return 1;
            }

            fill_random_matrix(A, n);
            fill_random_matrix(B, n);

            multiply_iterative(
                n,
                (matrix_value_t (*)[n]) A,
                (matrix_value_t (*)[n]) B,
                (matrix_value_t (*)[n]) reference
            );

            run_experiment(file, "iterative", multiply_iterative, n, pair_id, A, B, reference);
            run_experiment(file, "recursive", multiply_divide_conquer, n, pair_id, A, B, reference);
            run_experiment(file, "hybrid_divide_conquer", multiply_hybrid_divide_conquer_wrapper, n, pair_id, A, B, reference);
            run_experiment(file, "strassen", multiply_strassen, n, pair_id, A, B, reference);
            run_experiment(file, "hybrid_strassen", multiply_hybrid_strassen_wrapper, n, pair_id, A, B, reference);

            free(A);
            free(B);
            free(reference);
        }
    }

    fclose(file);

    printf("Experiments completed.\n");
    printf("Results saved to %s\n", results_file_path);

    return 0;
}
