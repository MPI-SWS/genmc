/*
 * Demonstrates that lock-repairing has to take place before backward
 * revisiting.  Otherwise, when T2:FAI tries to backward-revisit T1, T1
 * is dangling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;
pthread_mutex_t l;

void *thread_1(void *unused)
{
	atomic_fetch_add(&x, 1);
	pthread_mutex_lock(&l);
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread_2(void *unused)
{
	pthread_mutex_lock(&l);
	atomic_fetch_add(&x, 1);
	pthread_mutex_unlock(&l);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();

	return 0;
}
