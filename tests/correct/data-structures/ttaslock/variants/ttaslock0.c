#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>
#include <assert.h>

#include "../main.c"

int main()
{
	pthread_t t[N];

	ttaslock_init(&lock);
	for (int i = 0u; i < N; i++) {
		if (pthread_create(&t[i], NULL, thread_n, (void *) i))
			abort();
	}

	return 0;
}
