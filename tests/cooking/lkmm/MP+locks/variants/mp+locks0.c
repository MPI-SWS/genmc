#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x, y;
spinlock_t s;

void *P0(void *unused)
{
	int r1, r2;

	spin_lock(&s);
	r1 = READ_ONCE(x);
	spin_unlock(&s);
	spin_lock(&s);
	r2 = READ_ONCE(y);
	spin_unlock(&s);
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
