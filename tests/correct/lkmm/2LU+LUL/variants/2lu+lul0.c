#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
/* #include <stdatomic.h> */
#include <lkmm.h>
#include <genmc.h>

pthread_mutex_t l;
int x;

void *thread_1(void *unused)
{
	pthread_mutex_lock(&l);
	pthread_mutex_unlock(&l);

	pthread_mutex_lock(&l);
	pthread_mutex_unlock(&l);
	return NULL;
}

void *thread_2(void *unused)
{
	pthread_mutex_lock(&l);
	pthread_mutex_unlock(&l);

	pthread_mutex_lock(&l);
	/* pthread_mutex_unlock(&l); */
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
