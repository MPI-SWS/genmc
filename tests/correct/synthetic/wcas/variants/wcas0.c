#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../wcas.c"

int main()
{
	pthread_t t0, t[N];

	if (pthread_create(&t0, NULL, thread_0, NULL))
		abort();
	for (int i = 0; i < N; i++)
		if (pthread_create(&t[i], NULL, thread_n, NULL))
			abort();

	return 0;
}
