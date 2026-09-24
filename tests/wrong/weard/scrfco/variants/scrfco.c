#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

atomic_int x;
atomic_int y;
int a;

void *thread_1(void *unused)
{
	int r_x = atomic_load_explicit(&x, memory_order_seq_cst);
	int r_y = atomic_load_explicit(&y, memory_order_seq_cst);
	if (r_x == 2 && r_y == 0) {
		a = 1;
	}
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store_explicit(&x, 2, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_store_explicit(&y, 1, memory_order_seq_cst);
	atomic_store_explicit(&x, 1, memory_order_seq_cst);
	int r_x = atomic_load_explicit(&x, memory_order_relaxed);
	if (r_x == 2)
		a = 2;
	return NULL;
}

int main()
{
	pthread_t t1, t2, t3;

	atomic_store_explicit(&x, 0, memory_order_relaxed);
	atomic_store_explicit(&y, 0, memory_order_relaxed);

	pthread_create(&t1, NULL, thread_1, NULL);
	pthread_create(&t2, NULL, thread_2, NULL);
	pthread_create(&t3, NULL, thread_3, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	return 0;
}
