#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../mp-sc.c"

int main()
{
	pthread_t t1, t2;

	myinit(&x, 0);
	myinit(&y, 0);

	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();

	if (pthread_join(t2, NULL))
		abort();
	if (pthread_join(t1, NULL))
		abort();
}
