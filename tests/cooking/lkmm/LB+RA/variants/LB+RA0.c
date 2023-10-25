#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;
int s;

void *P0(void *unused)
{
	int r0 = READ_ONCE(x);
	smp_store_release(&s, 1);
	int r1 = smp_load_acquire(&s);
	WRITE_ONCE(y, r1);
	return NULL;
}

void *P1(void *unused)
{
	int r1 = READ_ONCE(y);
	smp_mb();
	WRITE_ONCE(x, 1);
	return NULL;
}

/* exists (0:r0=1 /\ 1:r1=0) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
