#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
spinlock_t s;

void *P0(void *unused)
{
	WRITE_ONCE(x, 1);
	smp_wmb();
	spin_lock(&s);
	spin_unlock(&s);
	return NULL;
}

void *P1(void *unused)
{
	int r0;
	int r1;

	r0 = spin_trylock(&s);
	smp_rmb();
	r1 = READ_ONCE(x);
	if (r0)
		spin_unlock(&s);
	return NULL;
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
