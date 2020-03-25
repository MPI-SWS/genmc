#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>

#include "../diff-thread-unlock.c"

int main()
{
	pthread_t t[N];

        pthread_mutex_init(&lock, NULL);
        pthread_mutex_lock(&lock);
        {
		int i= 0;
		pthread_create(&t[i], NULL, runLock, NULL);
		i++;
		pthread_create(&t[i], NULL, runUnlock, NULL);
		i++;
		assert (N == i);
	}
	for (intptr_t i = 0; i < N; i++)
		pthread_join(t[i], NULL);

	return 0;
}
