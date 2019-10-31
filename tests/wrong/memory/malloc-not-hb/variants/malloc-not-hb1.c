#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../malloc-not-hb.c"

int main()
{
	pthread_t t1, t2, t3;

	if (pthread_create(&t2, NULL, thread2, NULL))
		abort();
	if (pthread_create(&t1, NULL, thread1, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread3, NULL))
		abort();


	return 0;
}
