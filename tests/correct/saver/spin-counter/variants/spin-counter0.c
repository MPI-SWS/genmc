#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;

void *thread_1(void *unused)
{
	int *i = malloc(sizeof(int));
	int *l = malloc(sizeof(int));

	*l = 4;
	for (*i = 0u; *i < *l; (*i)++) {
		if (atomic_load_explicit(&x, memory_order_relaxed) == 42)
			break;
	}
	int r = x;
	/* no frees so that EC succeeds */
	return NULL;
}

void *thread_2(void *unused)
{
	x = 17;
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
