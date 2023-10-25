#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Never
 *
 * Demonstrates use of (strong-fence ; xbstar ; r-pre-bounded) in
 * wr-vis for detecting data races.
 */

int x;
int y;
int z;

void *P0(void *unused)
{
	x = 1;
	smp_mb();
	WRITE_ONCE(y, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r1 = READ_ONCE(y);
	WRITE_ONCE(z, r1);
	return NULL;
}

void *P2(void *unused)
{
	int r2;
	int r3;

	r2 = smp_load_acquire(&z);
	if (r2)
		r3 = x;
	return NULL;
}

/* exists (2:r2=1 /\ 2:r3=0) */

int main()
{
	pthread_t t0, t1, t2;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	return 0;
}
