#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../publish.c"

int main()
{
	pthread_t tr, tw;

	if (pthread_create(&tr, NULL, thread_reader, NULL))
		abort();
	if (pthread_create(&tw, NULL, thread_writer, NULL))
		abort();

	return 0;
}
