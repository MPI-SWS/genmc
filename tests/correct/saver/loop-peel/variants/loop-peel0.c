#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

#include "../loop-peel.c"

int main()
{
	pthread_t tw[DEFAULT_WRITERS], tr;

	if (pthread_create(&tr, NULL, thread_reader, NULL))
		abort();
	for (int i = 0u; i < DEFAULT_WRITERS; i++)
		if (pthread_create(&tw[i], NULL, thread_writer, NULL))
			abort();


	return 0;
}
