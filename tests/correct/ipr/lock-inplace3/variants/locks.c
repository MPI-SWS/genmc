#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

pthread_mutex_t l1;
pthread_mutex_t l2;
pthread_mutex_t l3;

void *thread_1(void *unused)
{
	pthread_mutex_lock(&l3);
	pthread_mutex_lock(&l1);
	pthread_mutex_lock(&l1);
	pthread_mutex_lock(&l3);
	return NULL;
}

void *thread_2(void *unused)
{
	pthread_mutex_lock(&l2);
	pthread_mutex_lock(&l3);
	pthread_mutex_lock(&l3);
	pthread_mutex_lock(&l2);
	return NULL;
}

void *thread_3(void *unused)
{
	pthread_mutex_lock(&l2);
	pthread_mutex_lock(&l1);
	pthread_mutex_lock(&l1);
	pthread_mutex_lock(&l2);
	return NULL;
}

int main()
{
	pthread_t t1, t2, t3;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_2, NULL))
		abort();
	if (pthread_create(&t2, NULL, thread_3, NULL))
		abort();

	return 0;
}
