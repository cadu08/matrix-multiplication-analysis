#include <stdio.h>
#include "utils.h"
#include "divide_conquer.h"

int main() {
    int n = 2;

    int A[2][2] = {
        {1, 2},
        {3, 4}
    };

    int B[2][2] = {
        {5, 6},
        {7, 8}
    };

    int C[2][2];

    double start = get_time();

    multiply_divide_conquer(n, A, B, C);

    double end = get_time();

    printf("Result matrix:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }

    printf("Elapsed time: %.9f seconds\n", end - start);

    return 0;
}