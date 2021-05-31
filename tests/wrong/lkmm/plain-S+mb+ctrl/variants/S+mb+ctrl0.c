#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Never DATARACE
 *
 * Demonstrate compiler write-to-read transformation for plain write.
 */

int x;
int z;

void *P0(void *unused)
{
	WRITE_ONCE(z, 1);
	smp_mb();
	WRITE_ONCE(x, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r1;

	r1 = READ_ONCE(x);
	if (r1 == 1)
		z = 0;
	return NULL;
}

/* exists (1:r1=1 /\ z=1) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
