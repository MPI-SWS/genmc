#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_uint *x;
atomic_int flag;

void *thread_1(void *unused)
{
	atomic_store_explicit(x, 1, memory_order_relaxed); /* hb-before the load in T2 */
	atomic_store_explicit(&flag, 1, memory_order_release);
	atomic_store_explicit(x, 2, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	if (atomic_load_explicit(&flag, memory_order_acquire)) {
		unsigned r = atomic_load_explicit(x, memory_order_relaxed);
		(void)r;
	}
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	x = malloc(sizeof(*x));

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();

	free(x);
	return 0;
}
