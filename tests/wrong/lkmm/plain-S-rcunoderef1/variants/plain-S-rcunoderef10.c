#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/* Result: Never DATARACE */

int a;
int b;

void *P0(void *unused)
{
	a = 1;
	synchronize_rcu();
	b = 2;
	return NULL;
}

void *P1(void *unused)
{
	rcu_read_lock();
	int r1 = a;
	if (r1 == 0)
		b = 1;
	rcu_read_unlock();
	return NULL;
}

/* locations [1:r1] */
/* exists (~b=2) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
