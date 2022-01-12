#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

#include "../atomic-ops.c"

int main()
{
	pthread_t t1;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();

	return 0;
}
