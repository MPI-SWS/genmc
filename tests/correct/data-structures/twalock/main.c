#ifndef N
# define N 2
#endif

#include "twalock.c"

int shared;

TWALOCK_ARRAY_DECL;
twalock_t lock = TWALOCK_INIT();

void *thread_n(void *arg)
{
	intptr_t index = ((intptr_t) arg);

	twalock_acquire(&lock);
	shared = index;
	int r = shared;
	assert(r == index);
	twalock_release(&lock);
	return NULL;
}
