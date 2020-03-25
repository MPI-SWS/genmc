#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "../stateful01-2.c"

int main()
{
	pthread_t  t1, t2;

	pthread_mutex_init(&ma, NULL);
	pthread_mutex_init(&mb, NULL);

	data1 = 10;
	data2 = 10;

	if (pthread_create(&t1, NULL, thread1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread2, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();

	if (data1 != 16 && data2 != 5)
		__VERIFIER_error();

	return 0;
}
