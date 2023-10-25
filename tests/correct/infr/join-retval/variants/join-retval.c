#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>
#include <assert.h>

atomic_int x;

void *thread_1(void *unused)
{
	return (void *)(intptr_t) x;
}

int main()
{
	pthread_t t1;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();

	x = 1;

	intptr_t retval;
	if (pthread_join(t1, (void *) &retval))
		abort();
	assert(retval <= 1);

	return 0;
}
