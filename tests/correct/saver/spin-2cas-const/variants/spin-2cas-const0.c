#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;

void *thread_1(void *unused)
{
	for (int i = 0u; i < 42; i++)
		x = i;
	return NULL;
}

void *thread_2(void *unused)
{
	for (;;) {
		int r = 0;
		if (r == 42) {
			if (!atomic_compare_exchange_strong(&x, &r, 42))
				continue;
		} else {
			if (!atomic_compare_exchange_strong(&x, &r, 17))
				continue;
		}
	}

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
