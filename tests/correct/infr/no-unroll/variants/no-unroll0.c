#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;

void *thread_1(void *unused)
{
	int a = x;
	return NULL;
}

void *thread_2(void *unused)
{
	x = 42;
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	for (int i = 0u; i < 42; i++)
		;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
