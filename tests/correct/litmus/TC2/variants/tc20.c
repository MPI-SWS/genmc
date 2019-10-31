#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	int r1 = atomic_load_explicit(&x, memory_order_relaxed);
	int r2 = atomic_load_explicit(&x, memory_order_relaxed);
	if (r1 == r2)
		atomic_store_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r3 = atomic_load_explicit(&y, memory_order_relaxed);
	atomic_store_explicit(&x, r3, memory_order_relaxed);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
