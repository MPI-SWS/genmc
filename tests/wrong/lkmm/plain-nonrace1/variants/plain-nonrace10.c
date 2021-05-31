#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;


/*
 * Result: Sometimes DATARACE
 *
 * P2()'s write can race with P0()'s plain read from y.  Plus P0()
 * doesn't order its write to y.
 */

void *P0(void *unused)
{
	int r1;
	int r2;

	r1 = READ_ONCE(x);
	smp_rmb();
	if (r1 == 1)
		r2 = y;
	WRITE_ONCE(y, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r3;

	r3 = READ_ONCE(y);
	WRITE_ONCE(x, r3);
	return NULL;
}

void *P2(void *unused)
{
	WRITE_ONCE(y, 2);
	return NULL;
}

/* exists (0:r1=1 /\ 1:r3=1) */

int main()
{
	pthread_t t0, t1, t2, t3;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	return 0;
}
