#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

/*
 * Horrible test case, but shows that we have to take the argument
 * into account as well when checking for symmetric threads.
 * (Loading ARG would not work since then SR would fail because
 * the graph would be different, even without checking the arguments.)
 */

#ifndef N
# define N 2
#endif

atomic_int x;
int a[N];

void *thread_n(void *arg)
{
	atomic_fetch_add_explicit(&x, (int) arg, memory_order_relaxed);
	return NULL;
}

int main()
{
	pthread_t t[N];

	for (int i = 0; i < N; i++) {
		a[i] = i;
		if (pthread_create(&t[i], NULL, thread_n, &a[i]))
			abort();
	}

	return 0;
}
