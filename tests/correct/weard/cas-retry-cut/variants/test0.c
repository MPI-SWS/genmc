/*
 * CAS retry loops block and wake threads mid-sample, re-executing read
 * positions; pruning must keep replayed positions and allocations sound.
 */
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>

#define N 100

atomic_int lock;
atomic_int sum;

void *run(void *arg)
{
	for (int i = 0; i < N; i++) {
		int *p = malloc(sizeof(int));
		*p = 1;

		int exp = 0;
		while (!atomic_compare_exchange_strong(&lock, &exp, 1))
			exp = 0;
		atomic_fetch_add_explicit(&sum, *p, memory_order_relaxed);
		atomic_store(&lock, 0);

		free(p);
	}
	return NULL;
}

int main()
{
	pthread_t t1, t2, t3;

	pthread_create(&t1, NULL, run, NULL);
	pthread_create(&t2, NULL, run, NULL);
	pthread_create(&t3, NULL, run, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	return 0;
}
