#include <stdio.h>
#include "utils.h"

int main() {
    volatile long sum = 0;

    double start = get_time();

    for (long i = 0; i < 100000000; i++) {
        sum += i;
    }

    double end = get_time();

    printf("Sum: %ld\n", sum);
    printf("Elapsed time: %f seconds\n", end - start);

    return 0;
}