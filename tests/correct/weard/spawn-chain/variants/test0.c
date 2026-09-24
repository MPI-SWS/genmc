/*
 * Threads spawn mid-run and park in joins along a chain; pruning must stay
 * sound and the run must terminate.
 */
#include <pthread.h>
#include <stdatomic.h>

#define DEPTH 6
#define N 200

atomic_int y;

void *spawn(void *arg)
{
	long d = (long)arg;

	for (int i = 0; i < N; i++)
		atomic_fetch_add_explicit(&y, 1, memory_order_acq_rel);
	if (d < DEPTH) {
		pthread_t t;
		pthread_create(&t, NULL, spawn, (void *)(d + 1));
		pthread_join(t, NULL);
	}
	return NULL;
}

int main(void)
{
	pthread_t t;

	pthread_create(&t, NULL, spawn, (void *)0L);
	pthread_join(t, NULL);
	return 0;
}
