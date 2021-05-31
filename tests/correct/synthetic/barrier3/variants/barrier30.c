#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

#ifndef N
# define N 2
#endif

pthread_barrier_t barrier;
atomic_int x;

void *thread_n1()
{
	int r = 0;
	while (!atomic_compare_exchange_strong(&x, &r, 1))
		r = 0;

	x = 0;
	return NULL;
}

void *thread_n2()
{
	pthread_barrier_wait(&barrier);
	return NULL;
}

int main()
{
	pthread_t t1[N], t2[N];

	pthread_barrier_init(&barrier, NULL, N);

	for (unsigned i = 0; i < N; i++) {
		if (pthread_create(&t1[i], NULL, thread_n1, NULL))
			abort();
		if (pthread_create(&t2[i], NULL, thread_n2, NULL))
			abort();
	}

	/* pthread_barrier_destroy(&barrier); */

	return 0;
}
