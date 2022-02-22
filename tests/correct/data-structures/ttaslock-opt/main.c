#include "ttaslock.c"

#ifndef N
# define N 2
#endif

int shared;
ttaslock_t lock;

void *thread_n(void *arg)
{
	intptr_t index = ((intptr_t) arg);

	ttaslock_acquire(&lock);
	shared = index;
	int r = shared;
	assert(r == index);
	ttaslock_release(&lock);
	return NULL;
}
