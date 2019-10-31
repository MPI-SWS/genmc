#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../access-freed2.c"

int main()
{
	pthread_t t1, t2;

	p = malloc(sizeof(atomic_int));
	atomic_init(p, 0);

	if (pthread_create(&t1, NULL, thread1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread2, NULL))
		abort();

	return 0;
}
