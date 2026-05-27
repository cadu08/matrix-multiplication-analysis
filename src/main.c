#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
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
#define NUM_ALGORITHMS 5
#define NUM_THRESHOLD_ALGORITHMS 2
#define NUM_THRESHOLDS 5
#define CSV_HEADER "algorithm,matrix_size,pair_id,time_seconds,memory_before_kb,memory_after_kb,memory_difference_kb,heap_current_bytes,heap_peak_bytes,heap_allocations,is_correct,max_abs_error,max_rel_error\n"
#define THRESHOLD_CSV_HEADER "algorithm,threshold,matrix_size,pair_id,time_seconds,memory_before_kb,memory_after_kb,memory_difference_kb,heap_current_bytes,heap_peak_bytes,heap_allocations,is_correct,max_abs_error,max_rel_error\n"

#ifndef NUM_SIZES
#define NUM_SIZES 9
#endif

#ifndef NUM_PAIRS
#define NUM_PAIRS 30
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
typedef void (*matrix_multiply_threshold_fn)(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n], int threshold);

typedef struct {
    int is_correct;
    matrix_value_t max_abs_error;
    matrix_value_t max_rel_error;
} validation_result_t;

typedef struct {
    const char *name;
    matrix_multiply_fn multiply;
} algorithm_config_t;

typedef struct {
    const char *name;
    matrix_multiply_threshold_fn multiply;
} threshold_algorithm_config_t;

#ifndef THRESHOLD_SWEEP
static const char *ALGORITHM_NAMES[NUM_ALGORITHMS] = {
    "iterative",
    "recursive",
    "hybrid_divide_conquer",
    "strassen",
    "hybrid_strassen"
};
#endif

#ifdef THRESHOLD_SWEEP
static const char *THRESHOLD_ALGORITHM_NAMES[NUM_THRESHOLD_ALGORITHMS] = {
    "hybrid_divide_conquer",
    "hybrid_strassen"
};
#endif

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

#ifndef THRESHOLD_SWEEP
static int file_exists_and_is_not_empty(const char *path) {
    struct stat file_stat;

    return stat(path, &file_stat) == 0 && file_stat.st_size > 0;
}

static int algorithm_index(const char *algorithm_name) {
    for (int i = 0; i < NUM_ALGORITHMS; i++) {
        if (strcmp(algorithm_name, ALGORITHM_NAMES[i]) == 0) {
            return i;
        }
    }

    return -1;
}

static int size_index(int n, int sizes[], int size_count) {
    for (int i = 0; i < size_count; i++) {
        if (sizes[i] == n) {
            return i;
        }
    }

    return -1;
}

static int mark_completed_result_line(
    char *line,
    int sizes[],
    int size_count,
    int completed[NUM_SIZES][NUM_PAIRS + 1][NUM_ALGORITHMS]
) {
    char *fields[13];
    int field_count = 0;

    for (
        char *field = strtok(line, ",\r\n");
        field != NULL && field_count < 13;
        field = strtok(NULL, ",\r\n")
    ) {
        fields[field_count] = field;
        field_count++;
    }

    if (field_count < 13) {
        return 0;
    }

    if (strcmp(fields[10], "1") != 0) {
        return 0;
    }

    int algorithm = algorithm_index(fields[0]);
    int n = atoi(fields[1]);
    int pair_id = atoi(fields[2]);
    int size = size_index(n, sizes, size_count);

    if (
        algorithm < 0 ||
        size < 0 ||
        pair_id < 1 ||
        pair_id > NUM_PAIRS
    ) {
        return 0;
    }

    completed[size][pair_id][algorithm] = 1;

    return 1;
}

static int load_completed_results(
    const char *results_file_path,
    int sizes[],
    int size_count,
    int completed[NUM_SIZES][NUM_PAIRS + 1][NUM_ALGORITHMS]
) {
    char line[1024];
    int completed_rows = 0;
    FILE *file = fopen(results_file_path, "r");

    if (file == NULL) {
        return 0;
    }

    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        completed_rows += mark_completed_result_line(
            line,
            sizes,
            size_count,
            completed
        );
    }

    fclose(file);

    return completed_rows;
}

static int pair_has_pending_algorithms(
    int size_index,
    int pair_id,
    int completed[NUM_SIZES][NUM_PAIRS + 1][NUM_ALGORITHMS]
) {
    for (int algorithm = 0; algorithm < NUM_ALGORITHMS; algorithm++) {
        if (!completed[size_index][pair_id][algorithm]) {
            return 1;
        }
    }

    return 0;
}
#endif

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

    fflush(file);

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

void run_threshold_experiment(
    FILE *file,
    const char *algorithm_name,
    matrix_multiply_threshold_fn multiply,
    int threshold,
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
        fprintf(stderr, "Error allocating threshold result matrix C\n");
        exit(1);
    }

    matrix_value_t (*C)[n] = (matrix_value_t (*)[n]) C_data;

    long memory_before = get_memory_usage_kb();
    heap_tracker_reset();
    double start = get_time();

    multiply(n, A, B, C, threshold);

    double end = get_time();
    heap_stats_t heap_stats = heap_tracker_get_stats();
    long memory_after = get_memory_usage_kb();
    validation_result_t validation = validate_against_reference(n, C, reference);

    fprintf(
        file,
        "%s,%d,%d,%d,%.9f,%ld,%ld,%ld,%zu,%zu,%zu,%d,%.17g,%.17g\n",
        algorithm_name,
        threshold,
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

    fflush(file);

    if (!validation.is_correct) {
        fprintf(
            stderr,
            "Validation failed for %s threshold %d, size %d, pair %d: max_abs_error=%.17g, max_rel_error=%.17g\n",
            algorithm_name,
            threshold,
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

#ifdef THRESHOLD_SWEEP
static int run_threshold_sweep(int argc, char *argv[]) {
    int sizes[] = {512, 1024, 2048};
    int thresholds[] = {16, 32, 64, 128, 256};
    int available_sizes = (int) (sizeof(sizes) / sizeof(sizes[0]));
    int available_thresholds = (int) (sizeof(thresholds) / sizeof(thresholds[0]));
    const char *results_file_path = DEFAULT_RESULTS_FILE;
    threshold_algorithm_config_t algorithms[NUM_THRESHOLD_ALGORITHMS] = {
        {THRESHOLD_ALGORITHM_NAMES[0], multiply_hybrid_divide_conquer},
        {THRESHOLD_ALGORITHM_NAMES[1], multiply_hybrid_strassen}
    };

    if (NUM_SIZES > available_sizes) {
        fprintf(stderr, "NUM_SIZES cannot exceed %d in threshold sweep mode\n", available_sizes);
        return 1;
    }

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [threshold_results_csv_path]\n", argv[0]);
        return 1;
    }

    if (argc == 2) {
        results_file_path = argv[1];
    }

    ensure_results_directory_exists();

    FILE *file = fopen(results_file_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening threshold results file: %s\n", results_file_path);
        return 1;
    }

    fprintf(file, THRESHOLD_CSV_HEADER);
    srand(FIXED_SEED);

    for (int s = 0; s < NUM_SIZES; s++) {
        int n = sizes[s];

        printf("Running threshold sweep for size %d x %d...\n", n, n);
        fflush(stdout);

        for (int pair_id = 1; pair_id <= NUM_PAIRS; pair_id++) {
            matrix_value_t *A = malloc(n * n * sizeof(matrix_value_t));
            matrix_value_t *B = malloc(n * n * sizeof(matrix_value_t));
            matrix_value_t *reference = malloc(n * n * sizeof(matrix_value_t));

            if (A == NULL || B == NULL || reference == NULL) {
                fprintf(stderr, "Error allocating threshold sweep matrices\n");
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

            for (int threshold_index = 0; threshold_index < available_thresholds; threshold_index++) {
                int threshold = thresholds[threshold_index];

                for (int algorithm = 0; algorithm < NUM_THRESHOLD_ALGORITHMS; algorithm++) {
                    run_threshold_experiment(
                        file,
                        algorithms[algorithm].name,
                        algorithms[algorithm].multiply,
                        threshold,
                        n,
                        pair_id,
                        A,
                        B,
                        reference
                    );
                }
            }

            free(A);
            free(B);
            free(reference);
        }
    }

    fclose(file);

    printf("Threshold sweep completed.\n");
    printf("Results saved to %s\n", results_file_path);

    return 0;
}
#endif

int main(int argc, char *argv[]) {
#ifdef THRESHOLD_SWEEP
    return run_threshold_sweep(argc, argv);
#else
    int sizes[] = {64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
    int available_sizes = (int) (sizeof(sizes) / sizeof(sizes[0]));
    const char *results_file_path = DEFAULT_RESULTS_FILE;
    int results_file_path_set = 0;
    int resume_enabled = 0;
    int completed[NUM_SIZES][NUM_PAIRS + 1][NUM_ALGORITHMS] = {{{0}}};
    algorithm_config_t algorithms[NUM_ALGORITHMS] = {
        {ALGORITHM_NAMES[0], multiply_iterative},
        {ALGORITHM_NAMES[1], multiply_divide_conquer},
        {ALGORITHM_NAMES[2], multiply_hybrid_divide_conquer_wrapper},
        {ALGORITHM_NAMES[3], multiply_strassen},
        {ALGORITHM_NAMES[4], multiply_hybrid_strassen_wrapper}
    };

    if (NUM_SIZES > available_sizes) {
        fprintf(stderr, "NUM_SIZES cannot exceed %d\n", available_sizes);
        return 1;
    }

    for (int arg = 1; arg < argc; arg++) {
        if (strcmp(argv[arg], "--resume") == 0) {
            resume_enabled = 1;
        } else if (!results_file_path_set) {
            results_file_path = argv[arg];
            results_file_path_set = 1;
        } else {
            fprintf(stderr, "Usage: %s [results_csv_path] [--resume]\n", argv[0]);
            return 1;
        }
    }

    ensure_results_directory_exists();

    if (resume_enabled) {
        int completed_rows = load_completed_results(
            results_file_path,
            sizes,
            NUM_SIZES,
            completed
        );

        printf("Resume mode enabled: %d completed rows loaded from %s\n", completed_rows, results_file_path);
    }

    int append_results = resume_enabled && file_exists_and_is_not_empty(results_file_path);
    FILE *file = fopen(results_file_path, append_results ? "a" : "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening results file: %s\n", results_file_path);
        return 1;
    }

    if (!append_results) {
        fprintf(file, CSV_HEADER);
    }

    srand(FIXED_SEED);

    for (int s = 0; s < NUM_SIZES; s++) {
        int n = sizes[s];

        printf("Running experiments for size %d x %d...\n", n, n);
        fflush(stdout);

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

            if (pair_has_pending_algorithms(s, pair_id, completed)) {
                multiply_iterative(
                    n,
                    (matrix_value_t (*)[n]) A,
                    (matrix_value_t (*)[n]) B,
                    (matrix_value_t (*)[n]) reference
                );

                for (int algorithm = 0; algorithm < NUM_ALGORITHMS; algorithm++) {
                    if (completed[s][pair_id][algorithm]) {
                        continue;
                    }

                    run_experiment(
                        file,
                        algorithms[algorithm].name,
                        algorithms[algorithm].multiply,
                        n,
                        pair_id,
                        A,
                        B,
                        reference
                    );

                    completed[s][pair_id][algorithm] = 1;
                }
            }

            free(A);
            free(B);
            free(reference);
        }
    }

    fclose(file);

    printf("Experiments completed.\n");
    printf("Results saved to %s\n", results_file_path);

    return 0;
#endif
}
