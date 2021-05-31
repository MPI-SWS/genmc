#include "spinlock.c"

#ifndef N
# define N 2
#endif

int shared;
spinlock_t lock;

void *thread_n(void *arg)
{
	intptr_t index = ((intptr_t) arg);

	spinlock_acquire(&lock);
	shared = index;
	int r = shared;
	assert(r == index);
	spinlock_release(&lock);
	return NULL;
}
