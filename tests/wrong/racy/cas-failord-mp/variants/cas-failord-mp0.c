/* This test has a race that only manifests is a CAS fails with relaxed ordering.
 * If the success orering is used, the race is masked. */

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int x;
int data;

void *writer(void *arg)
{
	data = 42;
	atomic_store_explicit(&x, 1, memory_order_release);
	return NULL;
}

void *reader(void *arg)
{
	int e = 2;
	atomic_compare_exchange_strong_explicit(&x, &e, 3, memory_order_acquire,
						memory_order_relaxed);
	if (e == 1)
		assert(data == 42);
	return NULL;
}

int main()
{
	pthread_t w, r;

	pthread_create(&w, NULL, writer, NULL);
	pthread_create(&r, NULL, reader, NULL);
	pthread_join(w, NULL);
	pthread_join(r, NULL);
	return 0;
}
