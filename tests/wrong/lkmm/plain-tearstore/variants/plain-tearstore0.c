#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Sometimes DATARACE
 *
 * Plain stores tear, so that concurrent stores to the same variable
 * result in a final value different than either value stored.
 */

int x = 0;

void *P0(void *unused)
{
	x = 6;
	return NULL;
}

void *P1(void *unused)
{
	x = 12;
	return NULL;
}

/* exists (~x=0 /\ ~x=6 /\ ~x=12) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
