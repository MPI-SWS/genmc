#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../malloc-not-hb2.c"

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread2, NULL))
		abort();


	return 0;
}
