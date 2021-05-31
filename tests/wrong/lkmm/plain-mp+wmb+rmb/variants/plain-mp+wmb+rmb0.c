#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Sometimes DATARACE
 *
 * P0()'s writes can be transformed to reads, and besides there are
 * data races all over the place.
 */

int x;
int y;

void *P0(void *unused)
{
	x = 1;
	smp_wmb();
	y = 1;
	return NULL;
}

void *P1(void *unused)
{
	int r0 = y;
	smp_rmb();
	int r1 = x;
	return NULL;
}

/* exists (~1:r0=0 /\ ~1:r1=1) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
