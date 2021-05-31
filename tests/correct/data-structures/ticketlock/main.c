#include "ticketlock.c"

#ifndef N
# define N 2
#endif

int shared;
ticketlock_t lock;

void *thread_n(void *arg)
{
	intptr_t index = ((intptr_t) arg);

	ticketlock_acquire(&lock);
	shared = index;
	int r = shared;
	assert(r == index);
	ticketlock_release(&lock);
	return NULL;
}
