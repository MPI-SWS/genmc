#include <stdlib.h>
#include <pthread.h>

#include "../lock-wxwy+rxry.c"

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread2, NULL))
		abort();

	return 0;
}
