#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Sometimes DATARACE
 *
 * Because the cycle is permitted, there is a data race.
 */

int x;
int y;

void *P0(void *unused)
{
	WRITE_ONCE(x, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r1 = READ_ONCE(x);
	WRITE_ONCE(y, r1);
	return NULL;
}

void *P2(void *unused)
{
	int r2 = READ_ONCE(y);
	smp_mb();
	x = 2;
	return NULL;
}

/* exists (1:r1=1 /\ 2:r2=1 /\ x=1) */

int main()
{
	pthread_t t0, t1, t2;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	return 0;
}
