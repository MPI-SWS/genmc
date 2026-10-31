#include <stdlib.h>
#include <pthread.h>

void *thread_1(void *unused)
{
	return NULL;
}

int main()
{
	pthread_t t1;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();

	/* Join a negative TID */
	pthread_join((pthread_t)-1, NULL);
	return 0;
}
