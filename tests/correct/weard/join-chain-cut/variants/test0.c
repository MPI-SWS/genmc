/*
 * Main parks in join while workers synchronize through RMWs; pruning must
 * not confuse the parked thread under a small window.
 */
#include <pthread.h>
#include <stdatomic.h>

#define N 1000

atomic_int c;

void *worker(void *arg)
{
	for (int i = 0; i < N; i++)
		atomic_fetch_add_explicit(&c, 1, memory_order_acq_rel);
	return NULL;
}

int main(void)
{
	pthread_t t[3];

	for (long i = 0; i < 3; i++)
		pthread_create(&t[i], NULL, worker, NULL);
	for (int i = 0; i < 3; i++)
		pthread_join(t[i], NULL);
	return 0;
}
