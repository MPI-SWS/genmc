#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

atomic_int x;

void *thread_n(void *unused)
{
	int r = x;
	x = r;
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread_n, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_n, NULL))
		abort();

	return 0;
}
