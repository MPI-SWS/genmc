#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
spinlock_t lo;

void *P0(void *unused)
{
	spin_lock(&lo);
	smp_mb__after_spinlock();
	WRITE_ONCE(x, 1);
	spin_unlock(&lo);
	return NULL;
}

void *P1(void *unused)
{
	int r1;
	int r2;
	int r3;

	r1 = smp_load_acquire(&x);
	r2 = spin_is_locked(&lo);
	r3 = spin_is_locked(&lo);
	return NULL;
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
