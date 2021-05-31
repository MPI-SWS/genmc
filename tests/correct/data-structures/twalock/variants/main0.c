#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>
#include <genmc.h>

#include "../main.c"

int main()
{
	pthread_t t[N];

	twalock_init(&lock);
	for (int i = 0u; i < N; i++)
		pthread_create(&t[i], NULL, thread_n, (void *) i);

	return 0;
}
