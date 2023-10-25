#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;

#ifndef N
# define N 4
#endif

void *thread_1(void *unused)
{
	__VERIFIER_assume(x == N);
	return NULL;
}

void *thread_2(void *unused)
{
	for (int i = 1; i <= N; i++)
		x = i;
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
