#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Sometimes
 *
 * P0()'s write to z can be reordered before its read from x, but that
 * cannot happen for P1()'s smp_store_release(), so if P0() executes
 * its then-clause, that of P1() muse already have completed.  Therefore,
 * there is no data race, but the "exists" clause can be satisfied.
 */

int x;
int y;
int z;

void *P0(void *unused)
{
	int r0;

	r0 = READ_ONCE(x);
	if (r0 == 1) {
		y = 0;
		/* Above can be replaced by: if ( *y != 0) *y = 0; */
		smp_wmb();
	}
	WRITE_ONCE(z, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r1 = READ_ONCE(z);
	if (r1 == 1) {
		smp_rmb();
		int r2 = READ_ONCE(y);
	}
	smp_store_release(&x, r1);
	return NULL;
}

/* exists (0:r0=1 /\ 1:r1=1) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
