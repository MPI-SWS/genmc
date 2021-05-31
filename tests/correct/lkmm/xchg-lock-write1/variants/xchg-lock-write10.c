#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int s;

void *P0(void *unused)
{
	int rlock;

	WRITE_ONCE(x, 1);
	smp_wmb();
	rlock = xchg_acquire(&s, 1);
	smp_store_release(&s, 0);
}

void *P1(void *unused)
{
	int r0;
	int r1;

	r0 = xchg_acquire(&s, 1);
	smp_rmb();
	r1 = READ_ONCE(x);
	if (r0 == 0)
		smp_store_release(&s, 0);
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
