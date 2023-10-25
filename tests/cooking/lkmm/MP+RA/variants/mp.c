#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x, y, s;

void *P0(void *unused)
{
	int r1, r2, r3;

	r1 = READ_ONCE(x);
	smp_store_release(&s, 1);
	r3 = smp_load_acquire(&s);
	r2 = READ_ONCE(y);
	return NULL;
}

void *P1(void *unused)
{
	WRITE_ONCE(y, 1);
	smp_wmb();
	WRITE_ONCE(x, 1);
	return NULL;
}

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
