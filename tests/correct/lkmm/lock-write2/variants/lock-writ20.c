#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;
spinlock_t s;

void *P0(void *unused)
{
	int r0;

	WRITE_ONCE(x, 1);
	smp_wmb();
	spin_lock(&s);
	r0 = READ_ONCE(y);
	spin_unlock(&s);
	return NULL;
}

void *P1(void *unused)
{
	int r1;

	WRITE_ONCE(y, 1);
	smp_mb();
	r1 = READ_ONCE(x);
	return NULL;
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
