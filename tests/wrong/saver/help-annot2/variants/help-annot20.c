#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;

void *thread_1(void *unused)
{
	int r = 0;
	__VERIFIER_CAS_helped_explicit(&x, &r, 42, memory_order_relaxed, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r = 0;
	__VERIFIER_CAS_helping_explicit(&x, &r, 42, memory_order_relaxed, memory_order_relaxed);
	return NULL;
}

void *thread_3(void *unused)
{
	x = 0;
	return NULL;
}

int main()
{
	pthread_t t1, t2, t3;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread_3, NULL))
		abort();

	return 0;
}
