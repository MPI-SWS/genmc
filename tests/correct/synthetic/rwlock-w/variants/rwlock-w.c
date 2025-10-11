#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

#ifndef N
 #define N 3
#endif

pthread_rwlock_t rwlock;
int na;

void *thread_n(void *unused)
{
	pthread_rwlock_rdlock(&rwlock);
	int r = na;
	pthread_rwlock_unlock(&rwlock);
	return NULL;
}

void *thread_w(void *unused)
{
	pthread_rwlock_wrlock(&rwlock);
	na = 42;
	pthread_rwlock_unlock(&rwlock);
	return NULL;
}

int main()
{
	pthread_t t[N], tw;

	for (int i = 0; i < N; i++) {
		if (pthread_create(&t[i], NULL, thread_n, NULL))
			abort();
	}
	if (pthread_create(&tw, NULL, thread_w, NULL))
		abort();

	return 0;
}
