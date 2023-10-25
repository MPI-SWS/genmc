#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;
spinlock_t mylock;

void *P0(void *unused)
{
	int r0;
	int r1;

	r0 = READ_ONCE(y);
	spin_lock(&mylock);
	r1 = READ_ONCE(x);
	spin_unlock(&mylock);
	return NULL;
}

void *P1(void *unused)
{
	spin_lock(&mylock);
	WRITE_ONCE(x, 1);
	spin_unlock(&mylock);
	WRITE_ONCE(y, 1);
	return NULL;
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
