#include <stdatomic.h>
#include <pthread.h>

atomic_int x;
atomic_int y;

void *run(void *unused)
{
	if (y)
		x = 1;
	return NULL;
}

int main()
{
	pthread_t t;

	pthread_create(&t, NULL, run, NULL);

	y = 1;
	while (!x)
		;

	pthread_join(t, NULL);

	return 0;
}
