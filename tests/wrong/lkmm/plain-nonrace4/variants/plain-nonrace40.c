#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Never DATARACE
 *
 * Standard RCU pattern with intervening store to memory, except that
 * it is to shared memory rather than a local.  Need at least a
 * WRITE_ONCE() and READ_ONCE() on the two accesses to tmp for this
 * to be forbidden.
 */

int a;
int b;

int *tmp;
int *x = &a;

void *P0(void *unused)
{
	b = 1;
	smp_store_release(&x, &b);
	return NULL;
}

void *P1(void *unused)
{
	int *r1 = READ_ONCE(x);
	tmp = r1;
	int *r2 = tmp;
	int r3 = *r2;
	return NULL;
}

/* exists (1:r1=b /\ 1:r3=0) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
