/*
 * The parent never joins and never synchronizes with the workers;
 * pruning must handle the ever-live thread gracefully.
 */
#include <pthread.h>
#include <stdatomic.h>

#define N 800

atomic_int x;
atomic_int y;

void *worker(void *arg)
{
	for (int i = 0; i < N; i++)
		atomic_fetch_add_explicit(&y, 1, memory_order_acq_rel);
	return NULL;
}

int main(void)
{
	pthread_t t[2];

	for (long i = 0; i < 2; i++)
		pthread_create(&t[i], NULL, worker, NULL);
	for (int i = 0; i < N; i++)
		atomic_load_explicit(&x, memory_order_relaxed);
	for (int i = 0; i < 2; i++)
		pthread_join(t[i], NULL);
	return 0;
}
