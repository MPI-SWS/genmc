#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;

void *thread_1(void *unused)
{
	int r = x;
	int q = r;
	atomic_compare_exchange_strong(&x, &q, 42);
	while (r != 17) {
		q = r + 1;
		r = x;
	}
	return NULL;
}

void *thread_2(void *unused)
{
	x = 17;
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
