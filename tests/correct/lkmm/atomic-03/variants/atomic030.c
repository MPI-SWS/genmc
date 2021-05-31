#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

atomic_t x = ATOMIC_INIT(0);

void *P0(void *unused)
{
	int r0 = atomic_cmpxchg(&x, 0, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r0 = atomic_cmpxchg(&x, 0, 2);
	return NULL;
}

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
