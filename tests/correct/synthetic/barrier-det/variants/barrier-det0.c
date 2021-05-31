/* Adapted from: http://pages.cs.wisc.edu/~travitch/pthreads_primer.html */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef THREADS
# define THREADS 2
#endif
#ifndef DIM
# define DIM 7
#endif

/* No need for atomics as these will not be shared */
int initial_matrix[DIM][DIM]; /* =  { { 1, 0, 2, -1 }, */
			      /* 	  { 3, 0, 0, 5 }, */
			      /* 	  { 2, 1, 4, -3 }, */
			      /* 	  { 1, 0, 5, 0 } }; */
int final_matrix[DIM][DIM];

pthread_barrier_t barr;

/* Calculates the dot product for a particular ROW and COL and stores it in DEST */
void dot_product(int row, int col, int source[DIM][DIM], int dest[DIM][DIM])
{
        int sum = 0;

        for (int i = 0; i < DIM; i++)
                sum += source[row][i] * source[i][col];

	dest[row][col] = sum;
	return;
}

/* Copied from: https://www.geeksforgeeks.org/determinant-of-a-matrix */
void cofactor(int matrix[DIM][DIM], int cofactors[DIM][DIM], int p, int q, int n)
{
	int i = 0, j = 0;

	for (int row = 0; row < n; row++) {
		for (int col = 0; col < n; col++) {
			if (row != p && col != q) {
				cofactors[i][j++] = matrix[row][col];

				if (j == n - 1) {
					j = 0;
					i++;
				}
			}
		}
	}
	return;
}

/* Avoiding Guassian triangulation; resursion instead */
int determinant(int matrix[DIM][DIM], int n)
{
	int cofactors[DIM][DIM];
	int sign = 1;
	int det = 0;

	if (n == 1)
		return matrix[0][0];

	for (int f = 0; f < n; f++) {
		cofactor(matrix, cofactors, 0, f, n);
		det += sign * matrix[0][f] * determinant(cofactors, n - 1);
		sign = -sign;
	}
	return det;
}

void *thread_worker(void *arg)
{
	int rank = (int) arg;

	for (int row = rank * DIM / THREADS; row < DIM && row < (rank + 1) * DIM / THREADS; ++row)
		for (int col = 0; col < DIM; ++col)
			dot_product(row, col, initial_matrix, final_matrix);

	/* Synchronization point */
	int rc = pthread_barrier_wait(&barr);
	if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
		printf("Could not wait on barrier\n");
		pthread_exit(NULL);
	}

	for (int row = rank * DIM / THREADS; row < DIM && row < (rank + 1) * DIM / THREADS; ++row)
		for (int col = 0; col < DIM; ++col)
			dot_product(row, col, final_matrix, initial_matrix);
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thr[THREADS];

	/* Barrier initialization */
	if (pthread_barrier_init(&barr, NULL, THREADS)) {
		printf("Could not create a barrier\n");
		return -1;
	}

	for (int i = 0; i < THREADS; ++i) {
		if (pthread_create(&thr[i], NULL, thread_worker, (void *) i)) {
			printf("Could not create thread %d\n", i);
			return -1;
		}
	}

	for (int i = 0; i < THREADS; ++i) {
		if (pthread_join(thr[i], NULL)) {
			printf("Could not join thread %d\n", i);
			return -1;
		}
	}

	printf("The determinant of M^4 = %d\n", determinant(initial_matrix, DIM));
	return 0;
}
