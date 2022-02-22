#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "../conf-cas.c"

int main()
{
	pthread_t t[N];

	for (int i = 0u; i < N; i++) {
		if (pthread_create(&t[i], NULL, thread_n, NULL))
			abort();
	}

	return 0;
}
