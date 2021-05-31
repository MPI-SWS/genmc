#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Never DATARACE
 *
 * Plain loads tear, so that a load from a variable that is being
 * concurrently stored to can result in a final value different than any
 * of the values stored.
 */

int x;

void *P0(void *unused)
{
	WRITE_ONCE(x, 6);
	return NULL;
}

void *P1(void *unused)
{
	WRITE_ONCE(x, 12);
	return NULL;
}

void *P2(void *unused)
{
	int r1 = x;
	return NULL;
}

/* exists (~2:r1=0 /\ ~2:r1=6 /\ ~2:r1=12) */

int main()
{
	pthread_t t0, t1, t2;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	return 0;
}
