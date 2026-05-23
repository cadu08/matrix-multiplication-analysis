#ifndef MATRIX_H
#define MATRIX_H

typedef double matrix_value_t;

void fill_random_matrix(matrix_value_t *matrix, int n);
void print_matrix(matrix_value_t *matrix, int n);
void write_matrix_to_file(const char *filename, matrix_value_t *matrix, int n);

#endif
