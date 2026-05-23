#include <stdio.h>
#include <stdlib.h>
#include "divide_conquer.h"
#include "heap_tracker.h"
#include "iterative.h"

static void free_submatrices(
    void *A11,
    void *A12,
    void *A21,
    void *A22,
    void *B11,
    void *B12,
    void *B21,
    void *B22,
    void *C11,
    void *C12,
    void *C21,
    void *C22,
    void *M1,
    void *M2
) {
    tracked_free(A11); tracked_free(A12); tracked_free(A21); tracked_free(A22);
    tracked_free(B11); tracked_free(B12); tracked_free(B21); tracked_free(B22);
    tracked_free(C11); tracked_free(C12); tracked_free(C21); tracked_free(C22);
    tracked_free(M1); tracked_free(M2);
}

static void add_matrix(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
}

/*
 * Divide-and-conquer matrix multiplication.
 * Time complexity: T(n) = 8T(n/2) + O(n^2), therefore O(n^3).
 * The tracked heap allocations capture the recursive submatrix footprint
 * independently from allocator page reuse, which supports controlled
 * empirical comparison of peak memory usage across input sizes.
 */
void multiply_divide_conquer(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]) {
    if (n == 1) {
        C[0][0] = A[0][0] * B[0][0];
        return;
    }

    int m = n / 2;

    matrix_value_t (*A11)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*A12)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*A21)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*A22)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));

    matrix_value_t (*B11)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*B12)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*B21)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*B22)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));

    matrix_value_t (*C11)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*C12)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*C21)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*C22)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));

    matrix_value_t (*M1)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*M2)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));

    if (
        A11 == NULL || A12 == NULL || A21 == NULL || A22 == NULL ||
        B11 == NULL || B12 == NULL || B21 == NULL || B22 == NULL ||
        C11 == NULL || C12 == NULL || C21 == NULL || C22 == NULL ||
        M1 == NULL || M2 == NULL
    ) {
        fprintf(stderr, "Error allocating submatrices for divide and conquer multiplication\n");
        free_submatrices(
            A11, A12, A21, A22,
            B11, B12, B21, B22,
            C11, C12, C21, C22,
            M1, M2
        );
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            A11[i][j] = A[i][j];
            A12[i][j] = A[i][j + m];
            A21[i][j] = A[i + m][j];
            A22[i][j] = A[i + m][j + m];

            B11[i][j] = B[i][j];
            B12[i][j] = B[i][j + m];
            B21[i][j] = B[i + m][j];
            B22[i][j] = B[i + m][j + m];
        }
    }

    multiply_divide_conquer(m, A11, B11, M1);
    multiply_divide_conquer(m, A12, B21, M2);
    add_matrix(m, M1, M2, C11);

    multiply_divide_conquer(m, A11, B12, M1);
    multiply_divide_conquer(m, A12, B22, M2);
    add_matrix(m, M1, M2, C12);

    multiply_divide_conquer(m, A21, B11, M1);
    multiply_divide_conquer(m, A22, B21, M2);
    add_matrix(m, M1, M2, C21);

    multiply_divide_conquer(m, A21, B12, M1);
    multiply_divide_conquer(m, A22, B22, M2);
    add_matrix(m, M1, M2, C22);

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            C[i][j] = C11[i][j];
            C[i][j + m] = C12[i][j];
            C[i + m][j] = C21[i][j];
            C[i + m][j + m] = C22[i][j];
        }
    }

    free_submatrices(
        A11, A12, A21, A22,
        B11, B12, B21, B22,
        C11, C12, C21, C22,
        M1, M2
    );
}
