#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

atomic_t x;
int y;

void *P0(void *unused)
{
	WRITE_ONCE(y, 1);
	smp_wmb();
	atomic_set(&x, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r1;

	atomic_inc(&x);
	smp_rmb();
	r1 = READ_ONCE(y);
	return NULL;
}

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
