#include <stdio.h>
#include <stdlib.h>

#include "heap_tracker.h"
#include "hybrid_strassen.h"
#include "iterative.h"

static void free_hybrid_strassen_matrices(
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
    void *M2,
    void *M3,
    void *M4,
    void *M5,
    void *M6,
    void *M7,
    void *T1,
    void *T2
) {
    tracked_free(A11); tracked_free(A12); tracked_free(A21); tracked_free(A22);
    tracked_free(B11); tracked_free(B12); tracked_free(B21); tracked_free(B22);
    tracked_free(C11); tracked_free(C12); tracked_free(C21); tracked_free(C22);
    tracked_free(M1); tracked_free(M2); tracked_free(M3); tracked_free(M4);
    tracked_free(M5); tracked_free(M6); tracked_free(M7);
    tracked_free(T1); tracked_free(T2);
}

static void add_matrix(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
}

static void subtract_matrix(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = A[i][j] - B[i][j];
        }
    }
}

/*
 * Hybrid Strassen matrix multiplication.
 * Time complexity: O(n^log2(7)) for fixed threshold, because the upper
 * recursive levels still follow T(n) = 7T(n/2) + O(n^2).
 * The threshold replaces small Strassen subproblems with the iterative
 * O(n^3) kernel, reducing recursion overhead, temporary allocations and
 * cancellation-prone additions where Strassen's asymptotic gain is least
 * likely to dominate practical costs.
 */
void multiply_hybrid_strassen(int n, matrix_value_t A[n][n], matrix_value_t B[n][n], matrix_value_t C[n][n], int threshold) {
    if (n <= threshold) {
        multiply_iterative(n, A, B, C);
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
    matrix_value_t (*M3)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*M4)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*M5)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*M6)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*M7)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));

    matrix_value_t (*T1)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));
    matrix_value_t (*T2)[m] = tracked_malloc(sizeof(matrix_value_t[m][m]));

    if (
        A11 == NULL || A12 == NULL || A21 == NULL || A22 == NULL ||
        B11 == NULL || B12 == NULL || B21 == NULL || B22 == NULL ||
        C11 == NULL || C12 == NULL || C21 == NULL || C22 == NULL ||
        M1 == NULL || M2 == NULL || M3 == NULL || M4 == NULL ||
        M5 == NULL || M6 == NULL || M7 == NULL ||
        T1 == NULL || T2 == NULL
    ) {
        fprintf(stderr, "Error allocating submatrices for hybrid Strassen multiplication\n");
        free_hybrid_strassen_matrices(
            A11, A12, A21, A22,
            B11, B12, B21, B22,
            C11, C12, C21, C22,
            M1, M2, M3, M4, M5, M6, M7,
            T1, T2
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

    add_matrix(m, A11, A22, T1);
    add_matrix(m, B11, B22, T2);
    multiply_hybrid_strassen(m, T1, T2, M1, threshold);

    add_matrix(m, A21, A22, T1);
    multiply_hybrid_strassen(m, T1, B11, M2, threshold);

    subtract_matrix(m, B12, B22, T2);
    multiply_hybrid_strassen(m, A11, T2, M3, threshold);

    subtract_matrix(m, B21, B11, T2);
    multiply_hybrid_strassen(m, A22, T2, M4, threshold);

    add_matrix(m, A11, A12, T1);
    multiply_hybrid_strassen(m, T1, B22, M5, threshold);

    subtract_matrix(m, A21, A11, T1);
    add_matrix(m, B11, B12, T2);
    multiply_hybrid_strassen(m, T1, T2, M6, threshold);

    subtract_matrix(m, A12, A22, T1);
    add_matrix(m, B21, B22, T2);
    multiply_hybrid_strassen(m, T1, T2, M7, threshold);

    add_matrix(m, M1, M4, T1);
    subtract_matrix(m, T1, M5, T2);
    add_matrix(m, T2, M7, C11);

    add_matrix(m, M3, M5, C12);
    add_matrix(m, M2, M4, C21);

    subtract_matrix(m, M1, M2, T1);
    add_matrix(m, T1, M3, T2);
    add_matrix(m, T2, M6, C22);

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            C[i][j] = C11[i][j];
            C[i][j + m] = C12[i][j];
            C[i + m][j] = C21[i][j];
            C[i + m][j + m] = C22[i][j];
        }
    }

    free_hybrid_strassen_matrices(
        A11, A12, A21, A22,
        B11, B12, B21, B22,
        C11, C12, C21, C22,
        M1, M2, M3, M4, M5, M6, M7,
        T1, T2
    );
}
