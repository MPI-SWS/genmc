#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	__VERIFIER_assume(x == 1);
	return NULL;
}

void *thread_2(void *unused)
{
	int r = y;
	return NULL;
}

void *thread_3(void *unused)
{
	x = 1;
	return NULL;
}

void *thread_4(void *unused)
{
	y = 1;
	return NULL;
}

int main()
{
	pthread_t t1, t2, t3, t4;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread_3, NULL))
		abort();
	if (pthread_create(&t4, NULL, thread_4, NULL))
		abort();

	return 0;
}
