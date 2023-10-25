#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../conf-reads.c"

int main()
{
	pthread_t tw, t[N];

	if (pthread_create(&tw, NULL, thread_writer, NULL))
		abort();
	for (int i = 0u; i < N; i++) {
		if (pthread_create(&t[i], NULL, thread_n, NULL))
			abort();
	}

	return 0;
}
