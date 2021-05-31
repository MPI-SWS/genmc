#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	while (1) {
	  if (x) break;
	  if (y) break;
        } ;
	return NULL;
}

void *thread_2(void *unused)
{
	if (!x) y = 1;
	return NULL;
}

void *thread_3(void *unused)
{
	if (!y) x = 1;
	return NULL;
}

int main()
{
	pthread_t t1, t2, t3;
	x = 0;
	y = 0;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	if (pthread_create(&t3, NULL, thread_3, NULL))
		abort();

	return 0;
}
