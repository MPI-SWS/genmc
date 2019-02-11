#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../jlt.c"

int main()
{
	pthread_t t1, t2, t3;

	if (pthread_create(&t2, NULL, thread_two, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread_three, NULL))
		abort();
	if (pthread_create(&t1, NULL, thread_one, NULL))
		abort();

	return 0;
}
