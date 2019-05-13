#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>

#include "../chase-lev.c"

int main()
{
	pthread_t t0, ts[DEFAULT_STEALERS];

	if (pthread_create(&t0, NULL, thread_pp, NULL))
		abort();
	for (int i = 0; i < DEFAULT_STEALERS; i++) {
		if (pthread_create(&ts[i], NULL, thread_s, NULL))
			abort();
	}

	return 0;
}
