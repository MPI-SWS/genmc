#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#ifndef THREADS
# define THREADS 2
#endif
#ifndef DIM
# define DIM 4
#endif

#define INF (999)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/* No need for atomics as these will not be shared */
int initial_matrix[DIM][DIM] = { { 0, 3, INF, 5 },
{ 2, 0, INF, 4 },
{ INF, 1, 0, INF },
{ INF, INF, 2, 0 } };

int helper_matrix[DIM][DIM] = { { INF, INF, INF, INF },
{ INF, INF, INF, INF },
{ INF, INF, INF, INF },
{ INF, INF, INF, INF } };

pthread_barrier_t barr;

void trans_closure(int row, int col, int source[DIM][DIM], int dest[DIM][DIM])
{
	int minK = INF;
	for (int k = 0; k < DIM; k++) {
		int sum = source[row][k] + source[k][col];
		minK = MIN(minK, sum);
	}
	dest[row][col] = MIN(source[row][col], minK);
	return;
}

void *thread_worker(void *arg)
{
	int rank = (int) arg;
	int dim = DIM + 1;

	do {
		for (int row = rank * DIM / THREADS; row < DIM && row < (rank + 1) * DIM / THREADS; ++row)
			for (int col = 0; col < DIM; ++col)
				trans_closure(row, col, initial_matrix, helper_matrix);

		/* Synchronize */
		int rc = pthread_barrier_wait(&barr);
		if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
			printf("Could not wait on barrier\n");
			pthread_exit(NULL);
		}

		for (int row = rank * DIM / THREADS; row < DIM && row < (rank + 1) * DIM / THREADS; ++row)
			for (int col = 0; col < DIM; ++col)
				trans_closure(row, col, helper_matrix, initial_matrix);

		/* Synchronize again */
		rc = pthread_barrier_wait(&barr);
		if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
			printf("Could not wait on barrier\n");
			pthread_exit(NULL);
		}
	} while ((dim >>= 1));
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

	/* printf("The transitive closure of M is:\n"); */
	/* for (int i = 0; i < DIM; i++) { */
	/* 	for (int j = 0; j < DIM; j++) */
	/* 		printf("%d ", initial_matrix[i][j]); */
	/* 	printf("\n"); */
	/* } */
	return 0;
}
