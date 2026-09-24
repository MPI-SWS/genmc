/*
 * Relaxed readers mix with RMW synchronization across threads; pruning
 * must keep the coherence lists consistent.
 */
#include <pthread.h>
#include <stdatomic.h>

#define N 800

atomic_int z;
atomic_int y;

void *lone(void *arg)
{
	atomic_store_explicit(&z, 7, memory_order_relaxed);
	return NULL;
}

void *reader(void *arg)
{
	atomic_load_explicit(&z, memory_order_relaxed);
	for (int i = 0; i < N; i++)
		atomic_fetch_add_explicit(&y, 1, memory_order_acq_rel);
	return NULL;
}

void *writers(void *arg)
{
	for (int i = 0; i < N; i++) {
		atomic_store_explicit(&z, i, memory_order_relaxed);
		atomic_fetch_add_explicit(&y, 1, memory_order_acq_rel);
	}
	return NULL;
}

int main(void)
{
	pthread_t l, r, w;

	pthread_create(&l, NULL, lone, NULL);
	pthread_create(&r, NULL, reader, NULL);
	pthread_create(&w, NULL, writers, NULL);

	pthread_join(l, NULL);
	pthread_join(r, NULL);
	pthread_join(w, NULL);
	return 0;
}
