#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int x;
atomic_int y;
int a;

void *thread_one(void *arg)
{
	atomic_store_explicit(&x, 1, memory_order_release);
	int r_y = atomic_load_explicit(&y, memory_order_acquire);
    if (r_y == 0) {
        a = 1;
    }
	return NULL;
}

void *thread_two(void *arg)
{
	atomic_store_explicit(&y, 1, memory_order_release);
	int r_x = atomic_load_explicit(&x, memory_order_acquire);
    if (r_x == 0) {
        a = 2;
    }
	return NULL;
}

int main()
{
	pthread_t t1, t2;

    atomic_store_explicit(&x, 0, memory_order_relaxed);
    atomic_store_explicit(&y, 0, memory_order_relaxed);

	if (pthread_create(&t1, NULL, thread_one, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_two, NULL))
		abort();

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

	return 0;
}
