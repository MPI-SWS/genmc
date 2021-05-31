#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

#ifndef THREADS
# define THREADS 2
#endif

#ifndef MAX
# define MAX 4
#endif

#ifndef DEF
# define DEF (-42)
#endif

pthread_mutex_t l;
int a[MAX] = { [0 ... MAX-1] = DEF };

int check_array(int array[MAX])
{
	for (int i = 0u; i < MAX; i++) {
		if (a[i] == DEF)
			return 0;
	}
	return 1;
}

void *thread_checker(void *unused)
{
	while (1) {
		/* Check whether all threads have finished */
		pthread_mutex_lock(&l);
		if (check_array(a)) {
			pthread_mutex_unlock(&l);
			break;
		}
		pthread_mutex_unlock(&l);
	}
	return NULL;
}

void *thread_worker(void *arg)
{
	intptr_t index = (intptr_t) arg;

	for (intptr_t i = index * MAX / THREADS; i < MAX && i < (index + 1) * MAX / THREADS; ++i) {
		/* Do some work */
		int result = i;

		/* Write result to its place */
		pthread_mutex_lock(&l);
		a[i] = result;
		pthread_mutex_unlock(&l);
	}
	return NULL;
}

int main()
{
	pthread_t tc, tw[THREADS];

	if (pthread_create(&tc, NULL, thread_checker, NULL))
		abort();
	for (int i = 0u; i < THREADS; i++) {
		if (pthread_create(&tw[i], NULL, thread_worker, (void *) i))
			abort();
	}

	return 0;
}
