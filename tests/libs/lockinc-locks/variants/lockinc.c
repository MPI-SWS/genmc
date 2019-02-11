#include <stdlib.h>
#include <pthread.h>

#include "../lockinc.c"

int main()
{
	pthread_t t[N];

	myinit(&l, 0);

	for (int i = 0; i < N; i++)
		if (pthread_create(&t[i], NULL, thread_n, NULL))
			abort();

	for (int i = 0; i < N; i++)
		if (pthread_join(t[i], NULL))
			abort();

	return 0;
}
