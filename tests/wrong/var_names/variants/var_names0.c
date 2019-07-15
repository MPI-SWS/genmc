#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

#include "../var_names.c"

int main()
{
	pthread_t t1, t2;

	struct foo shared;

	if (pthread_create(&t1, NULL, thread_1, &shared))
		abort();
	if (pthread_create(&t2, NULL, thread_2, &shared))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();

	return 0;
}
