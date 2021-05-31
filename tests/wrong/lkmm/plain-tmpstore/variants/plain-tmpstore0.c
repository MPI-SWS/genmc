#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Sometimes DATARACE
 *
 * Plain stores can be preceded by use of the stored location as temporary
 * storage.
 */

int x;

void *P0(void *unused)
{
	x = 0;
	return NULL;
}

void *P1(void *unused)
{
	int r1 = READ_ONCE(x);
	return NULL;
}

/* exists (~1:r1=0) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
