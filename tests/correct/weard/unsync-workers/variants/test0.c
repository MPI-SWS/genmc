/*
 * Workers never synchronize; cutting must terminate cleanly.
 */
#include <pthread.h>
#include <stdatomic.h>

#define N 800

atomic_int loc[3];

void *worker(void *arg)
{
	long id = (long)arg;

	for (int i = 0; i < N; i++)
		atomic_store_explicit(&loc[id], i, memory_order_relaxed);
	return NULL;
}

int main(void)
{
	pthread_t t[3];

	for (long i = 0; i < 3; i++)
		pthread_create(&t[i], NULL, worker, (void *)i);
	for (int i = 0; i < 3; i++)
		pthread_join(t[i], NULL);
	return 0;
}
