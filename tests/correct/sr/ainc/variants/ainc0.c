#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

/*
 * Symmetry reduction cannot be applied for this one, due to memory
 * accesses generated between the creation of the threads.
 * These memory accesses are created to access the t[N] array.
 */

#ifndef N
# define N 2
#endif

atomic_int x;

void *thread_n(void *unused)
{
	atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

int main()
{
	pthread_t t[N];

	t[0] = __VERIFIER_spawn(thread_n, NULL);
	for (int i = 1; i < N; i++) {
		t[i] = __VERIFIER_spawn_symmetric(thread_n, NULL, t[i-1]);
	}

	return 0;
}
