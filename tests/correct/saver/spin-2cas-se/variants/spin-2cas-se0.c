#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

/*
 * In this test case one path leadins to the loop's only latch
 * potentially has side effects, but we can still validate
 * that this is a spinloop dynamically
*/

atomic_int x;

void *thread_1(void *unused)
{
	int r;

	r = atomic_load_explicit(&x, memory_order_seq_cst);
	for (;;) {
		if (r == 42) {
			if (atomic_compare_exchange_strong(&x, &r, 42))
				break;
		} else {
			atomic_compare_exchange_strong(&x, &r, 17);
		}
	}
	return NULL;
}

void *thread_2(void *unused)
{
	x = 42;
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
