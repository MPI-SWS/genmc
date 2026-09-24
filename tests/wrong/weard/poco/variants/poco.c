#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

atomic_int x;
atomic_int y;
int a;

void *thread_1(void *unused)
{
	atomic_store_explicit(&x, 2, memory_order_relaxed);
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	int r_y = atomic_load_explicit(&y, memory_order_relaxed);
	if (r_y == 2)
		a = 1;
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&y, 2, memory_order_relaxed);
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	int r_x = atomic_load_explicit(&x, memory_order_relaxed);
	if (r_x == 2)
		a = 2;
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	pthread_create(&t1, NULL, thread_1, NULL);
	pthread_create(&t2, NULL, thread_2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	return 0;
}
