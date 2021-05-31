#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

/* Validates that we only do DSA for iterations of the same
 * spinloop */

atomic_int x;
atomic_int y;

void spin_until_zero(atomic_int *v)
{
	while (atomic_load_explicit(v, memory_order_relaxed)) {
		/* This will not produce side-effects */
		int r = 17;
		atomic_compare_exchange_strong(&y, &r, 42);
	}
	return;
}

void *thread_1(void *unused)
{
	/* spin once... */
	spin_until_zero(&x);

	/* spin twice! */
	spin_until_zero(&x);
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
