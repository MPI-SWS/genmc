#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;
int z;
spinlock_t mylock;

void *P0(void *unused)
{
	spin_lock(&mylock);
	WRITE_ONCE(x, 1);
	WRITE_ONCE(y, 1);
	spin_unlock(&mylock);
	return NULL;
}

void *P1(void *unused)
{
	int r0;

	spin_lock(&mylock);
	r0 = READ_ONCE(y);
	WRITE_ONCE(z, 1);
	spin_unlock(&mylock);
	return NULL;
}

void *P2(void *unused)
{
	int r1;
	int r2;

	r2 = READ_ONCE(z);
	smp_mb();
	r1 = READ_ONCE(x);
	return NULL;
}

int main()
{
	pthread_t t0, t1, t2;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	return 0;
}
