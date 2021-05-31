#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

#include "../loop-peel2.c"

int main()
{
	pthread_t tw, tr[DEFAULT_READERS];

	if (pthread_create(&tw, NULL, thread_writer, NULL))
		abort();
	for (int i = 0u; i < DEFAULT_READERS; i++) {
		if (pthread_create(&tr[i], NULL, thread_reader, NULL))
			abort();
	}

	return 0;
}
