#include <stdio.h>
#include "utils.h"
#include "hybrid.h"

int main() {
    int n = 4;
    int threshold = 2;

    int A[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };

    int B[4][4] = {
        {1, 0, 2, 1},
        {0, 1, 1, 2},
        {2, 1, 0, 1},
        {1, 2, 1, 0}
    };

    int C[4][4];

    double start = get_time();

    multiply_hybrid(n, A, B, C, threshold);

    double end = get_time();

    printf("Hybrid result matrix:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }

    printf("Threshold: %d\n", threshold);
    printf("Elapsed time: %.9f seconds\n", end - start);

    return 0;
}