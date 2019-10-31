#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>

#include "../SB+assert.c"

int main(int argc, char **argv)
{
	pthread_t t0, t1;

	if (pthread_create(&t0, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t1, NULL, thread_2, NULL))
		abort();

	if (pthread_join(t0, NULL))
		abort();
	if (pthread_join(t1, NULL))
		abort();

	assert(!(r_x == 0 && r_y == 0));

	return 0;
}
