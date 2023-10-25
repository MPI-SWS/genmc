#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

atomic_t x;
atomic_t y;

void *P0(void *unused)
{
	int r0;

	atomic_inc(&x);
	r0 = atomic_read(&y);
	return NULL;
}

void *P1(void *unused)
{
	int r1;

	atomic_inc(&y);
	r1 = atomic_read(&x);
	return NULL;
}

void *P2(void *unused)
{
	atomic_add(2, &x);
	atomic_add(2, &y);
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
