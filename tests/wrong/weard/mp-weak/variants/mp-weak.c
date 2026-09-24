#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

int a;
atomic_int x;
atomic_int y;

void *thread_1(void *arg)
{
	a = 1;
    atomic_store_explicit(&x, 1, memory_order_relaxed);
	atomic_store_explicit(&y, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *arg)
{
	int r_y = atomic_load_explicit(&y, memory_order_relaxed);
    int r_x = atomic_load_explicit(&x, memory_order_relaxed);

	if (r_y == 1 && r_x == 0) {
        a = 2;
    }
	return NULL;
}

int main()
{
	pthread_t t1, t2;

    atomic_store_explicit(&x, 0, memory_order_relaxed);
	atomic_store_explicit(&y, 0, memory_order_relaxed);
	pthread_create(&t1, NULL, thread_1, NULL);
	pthread_create(&t2, NULL, thread_2, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	return 0;
}