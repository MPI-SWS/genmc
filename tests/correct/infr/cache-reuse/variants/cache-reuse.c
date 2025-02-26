#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

/*
 * Designed to demonstrate that we don't have to track
 * allocations in our prefix cache.
 *
 * T1 writes N values.
 * T2 can reads a value, allocates an address, and
 * stores it into a pointer.
 * T3 reads the pointer (assumes it's non-NULL), and
 * then performs a long sequential computation.
 *
 * There are N different values to read, and the old
 * allocator would allocate a different address for
 * each address. If we always allocate the same address
 * for a given position, we can speed up the runtime:
 * the prefix of T3 will not change, and so we can
 * use the cache to determine the next events.
 */

#ifndef N
# define N 2 /* use large N to see speedup */
#endif

atomic_int x;
_Atomic(int *) p;

void *thread_1(void *unused)
{
	for (int i = 0U; i < N; i++) {
		x = i;
	}
	return NULL;
}

void *thread_2(void *unused)
{
	int a = x;
	p = (int *) malloc(sizeof(int));
	return NULL;
}

void *thread_3(void *unused)
{
	int *b = p;
	__VERIFIER_assume(b);

	/* Long piece of sequential code */
	for (int i = 0U; i < 100000; i++)
		;

	free(b);
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
