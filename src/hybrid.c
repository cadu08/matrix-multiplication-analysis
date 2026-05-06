#include <stdlib.h>
#include "hybrid.h"
#include "iterative.h"

static void add_matrix(int n, int A[n][n], int B[n][n], int C[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = A[i][j] + B[i][j];
        }
    }
}

void multiply_hybrid(int n, int A[n][n], int B[n][n], int C[n][n], int threshold) {
    if (n <= threshold) {
        multiply_iterative(n, A, B, C);
        return;
    }

    int m = n / 2;

    int (*A11)[m] = malloc(sizeof(int[m][m]));
    int (*A12)[m] = malloc(sizeof(int[m][m]));
    int (*A21)[m] = malloc(sizeof(int[m][m]));
    int (*A22)[m] = malloc(sizeof(int[m][m]));

    int (*B11)[m] = malloc(sizeof(int[m][m]));
    int (*B12)[m] = malloc(sizeof(int[m][m]));
    int (*B21)[m] = malloc(sizeof(int[m][m]));
    int (*B22)[m] = malloc(sizeof(int[m][m]));

    int (*C11)[m] = malloc(sizeof(int[m][m]));
    int (*C12)[m] = malloc(sizeof(int[m][m]));
    int (*C21)[m] = malloc(sizeof(int[m][m]));
    int (*C22)[m] = malloc(sizeof(int[m][m]));

    int (*M1)[m] = malloc(sizeof(int[m][m]));
    int (*M2)[m] = malloc(sizeof(int[m][m]));

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

    multiply_hybrid(m, A11, B11, M1, threshold);
    multiply_hybrid(m, A12, B21, M2, threshold);
    add_matrix(m, M1, M2, C11);

    multiply_hybrid(m, A11, B12, M1, threshold);
    multiply_hybrid(m, A12, B22, M2, threshold);
    add_matrix(m, M1, M2, C12);

    multiply_hybrid(m, A21, B11, M1, threshold);
    multiply_hybrid(m, A22, B21, M2, threshold);
    add_matrix(m, M1, M2, C21);

    multiply_hybrid(m, A21, B12, M1, threshold);
    multiply_hybrid(m, A22, B22, M2, threshold);
    add_matrix(m, M1, M2, C22);

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            C[i][j] = C11[i][j];
            C[i][j + m] = C12[i][j];
            C[i + m][j] = C21[i][j];
            C[i + m][j + m] = C22[i][j];
        }
    }

    free(A11); free(A12); free(A21); free(A22);
    free(B11); free(B12); free(B21); free(B22);
    free(C11); free(C12); free(C21); free(C22);
    free(M1); free(M2);
}