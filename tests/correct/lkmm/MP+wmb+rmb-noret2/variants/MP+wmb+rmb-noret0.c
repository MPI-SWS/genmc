#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
atomic_t y;

void *P0(void *unused)
{
	atomic_set(&y, 1);
	smp_wmb();
	WRITE_ONCE(x, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r1;

	r1 = READ_ONCE(x);
	smp_rmb();
	atomic_inc(&y);
	return NULL;
}

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
