#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

atomic_t x = ATOMIC_INIT(0);
atomic_t y = ATOMIC_INIT(0);

void *P0(void *unused)
{
	int r0;
	int r1;

	r0 = atomic_cmpxchg(&x, 0, 1);
	r1 = atomic_read(&y);
	return NULL;
}

void *P1(void *unused)
{
	int r0; int r1;
	r0 =  atomic_cmpxchg(&y, 0, 1) ;
	r1 = atomic_read(&x);
	return NULL;
}

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
