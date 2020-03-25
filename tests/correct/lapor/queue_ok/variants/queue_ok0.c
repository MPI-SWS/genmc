#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>

#include "../queue_ok.c"

int main(void)
{
	pthread_t id1, id2;

	enqueue_flag = true;
	dequeue_flag = false;

	init(&queue);

	if (empty(&queue) != EMPTY)
		__VERIFIER_error();

	if (pthread_mutex_init(&m, NULL))
		abort();

	if (pthread_create(&id1, NULL, t1, &queue))
		abort();
	if (pthread_create(&id2, NULL, t2, &queue))
		abort();

	if (pthread_join(id1, NULL))
		abort();
	if (pthread_join(id2, NULL))
		abort();

	return 0;
}
