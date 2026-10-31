/*
 * Assume the race on x found with the following schedule: z=2,x=1,x=2.
 * If the replay schedule ends before reaching the race, the scheduler
 * is free to run T2 past the original graph, and eventually hit a
 * different race (on z).
 */
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

int x, z;
atomic_int w;

void *thread_1(void *unused)
{
	z = 2;
	return NULL;
}

void *thread_2(void *unused)
{
	x = 1;
	z = 1;
	atomic_store_explicit(&w, 1, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	x = 2;
	return NULL;
}

int main()
{
	pthread_t t1, t2, t3;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread_3, NULL))
		abort();

	return 0;
}
