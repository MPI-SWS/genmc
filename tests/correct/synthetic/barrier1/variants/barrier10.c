#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#ifndef N
# define N 2
#endif

pthread_barrier_t barrier;

void *thread_n()
{
	pthread_barrier_wait(&barrier);
	return NULL;
}

int main()
{
	pthread_t t[N];

	pthread_barrier_init(&barrier, NULL, N);

	for (unsigned i = 0; i < N; i++) {
		if (pthread_create(&t[i], NULL, thread_n, NULL))
			abort();
	}

	/* pthread_barrier_destroy(&barrier); */

	return 0;
}
