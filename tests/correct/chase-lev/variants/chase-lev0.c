#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>

#include "../chase-lev.c"

int main()
{
	pthread_t t0, t1, t2;

	if (pthread_create(&t0, NULL, thread_0, NULL))
		abort();
	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
