#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;
atomic_int y;

pthread_barrier_t barrier;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	if (atomic_load_explicit(&y, memory_order_relaxed))
		if (!atomic_load_explicit(&x, memory_order_relaxed))
			pthread_barrier_init(&barrier, NULL, 17);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	pthread_barrier_init(&barrier, NULL, 42);

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
