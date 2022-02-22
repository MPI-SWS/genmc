#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;

void *thread_1(void *unused)
{
	int r = 0;
	__VERIFIER_helped_CAS(
		atomic_compare_exchange_strong_explicit(&x, &r, 42, memory_order_relaxed, memory_order_relaxed);
	);
	return NULL;
}

void *thread_2(void *unused)
{
	int r = 0;
	__VERIFIER_helping_CAS(
		atomic_compare_exchange_strong_explicit(&x, &r, 42, memory_order_relaxed, memory_order_relaxed);
	);
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
