#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../coa.c"

int main()
{
	pthread_t t1, t2, t[N];

	if (pthread_create(&t1, NULL, thread_one, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_two, NULL))
		abort();
	for (int i = 0; i < N - 1; i++) {
		idx[i] = i;
		if (pthread_create(&t[i], NULL, thread_n, &idx[i]))
			abort();
	}

	return 0;
}
