#include "mutex.c"

#ifndef N
# define N 2
#endif

int shared;
mutex_t mutex;

void *thread_n(void *arg)
{
	intptr_t index = ((intptr_t) arg);

	mutex_lock(&mutex);
	shared = index;
	int r = shared;
	assert(r == index);
	mutex_unlock(&mutex);
	return NULL;
}
