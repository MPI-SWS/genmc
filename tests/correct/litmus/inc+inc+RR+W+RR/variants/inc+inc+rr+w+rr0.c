#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

#include "../inc+inc+rr+w+rr.c"

int main()
{
	pthread_t t1, t2, t3, t4, t5;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread_3, NULL))
		abort();
	if (pthread_create(&t4, NULL, thread_4, NULL))
		abort();
	if (pthread_create(&t5, NULL, thread_5, NULL))
		abort();


	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();
	if (pthread_join(t3, NULL))
		abort();
	if (pthread_join(t4, NULL))
		abort();
	if (pthread_join(t5, NULL))
		abort();

	assert (!(a == 42 && b == 2 && c == 1 && d == 42));

	return 0;
}
