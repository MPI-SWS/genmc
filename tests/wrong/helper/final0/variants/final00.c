#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;

void *thread_1(void *unused)
{
	__VERIFIER_final_write(x = 1);
	return NULL;
}

void *thread_2(void *unused)
{
	x = 2;
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
