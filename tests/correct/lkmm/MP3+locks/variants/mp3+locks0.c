#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int r1, r2, r3;

int x, y;
spinlock_t s;

void *P0(void *unused)
{
	spin_lock(&s);
	WRITE_ONCE(x, 1);
	spin_unlock(&s);
	return NULL;
}

void *P1(void *unused)
{
	spin_lock(&s);
	r1 = READ_ONCE(x);
	WRITE_ONCE(y, 1);
	spin_unlock(&s);
	return NULL;
}

void *P2(void *unused)
{
	r2 = READ_ONCE(y);
	smp_rmb();
	r3 = READ_ONCE(x);
	return NULL;
}

int main()
{
	pthread_t t0, t1, t2;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	pthread_join(t0, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	assert(!(r1 == 1 && r2 == 1 && r3 == 0));

	return 0;
}
