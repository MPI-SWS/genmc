#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Sometimes DATARACE
 *
 * Plain stores can be preceded by use of the stored location as temporary
 * storage, which can result in classic out-of-thin-air values.
 * Note:  Involves data race.
 */

int x = 0;
int y = 0;

void *P0(void *unused)
{
	int r1 = x;
	y = r1;
	return NULL;
}

void *P1(void *unused)
{
	int r1 = y;
	x = r1;
	return NULL;
}

/* exists (~0:r1=0 \/ ~1:r1=0) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
