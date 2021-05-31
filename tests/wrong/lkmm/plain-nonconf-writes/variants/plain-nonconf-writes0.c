#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Sometimes DATARACE
 *
 * First, smp_wmb() isn't cumulative, so the result is allowed even when
 * all plain writes are convered to WRITE_ONCE().  Second, the read-to-write
 * optimization means that smp_wmb() does not necessarily order
 * plain writes.
 */

int x;
int y;

void *P0(void *unused)
{
	x = 1;
	smp_wmb();
	WRITE_ONCE(y, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r1;

	r1 = READ_ONCE(y);
	if (r1 == 1) {
		WRITE_ONCE(x, 2);
		smp_wmb();
		WRITE_ONCE(y, 2);
	}
	return NULL;
}

void *P2(void *unused)
{
	int r2;
	int r3;

	r2 = READ_ONCE(y);
	if (r2 == 2)
		x = 3;
	return NULL;
}

/* exists (1:r1=1 /\ 2:r2=2 /\ x=3) */

int main()
{
	pthread_t t0, t1, t2;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	return 0;
}
