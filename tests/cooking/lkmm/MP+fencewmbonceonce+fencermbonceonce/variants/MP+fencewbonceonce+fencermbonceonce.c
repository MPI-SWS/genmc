#include <stdlib.h>
#include <pthread.h>
#include <lkmm.h>
#include <assert.h>

int x;
int y;

int r0, r1;

void *P0(void *unused)
{
	WRITE_ONCE(x, 1);
	smp_wmb();
	WRITE_ONCE(y, 1);
	return NULL;
}

void *P1(void *unused)
{
	r0 = READ_ONCE(y);
	smp_rmb();
	r1 = READ_ONCE(x);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, P0, NULL))
		abort();
	if (pthread_create(&t2, NULL, P1, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();

	assert(!(r0 == 1 && r1 == 0));

	return 0;
}
