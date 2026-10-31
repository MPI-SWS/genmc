#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

#include "../skeleton-create.c"

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	int r0 = atomic_load_explicit(&y, memory_order_relaxed);
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	atomic_store_explicit(&x, 1, memory_order_relaxed);

	return 0;
}
