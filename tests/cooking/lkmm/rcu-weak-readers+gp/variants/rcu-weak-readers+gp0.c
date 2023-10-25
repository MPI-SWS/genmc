#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;
int z;

void *P0(void *unused)
{
	rcu_read_lock();
	int r1 = READ_ONCE(y);
	WRITE_ONCE(x, 1);
	rcu_read_unlock();
	return NULL;
}

void *P1(void *unused)
{
	int r2 = READ_ONCE(x);
	synchronize_rcu();
	WRITE_ONCE(z, 1);
	return NULL;
}

void *P2(void *unused)
{
	rcu_read_lock();
	int r3 = READ_ONCE(z);
	WRITE_ONCE(y, 1);
	rcu_read_unlock();
	return NULL;
}

int main()
{
	pthread_t t0, t1, t2;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	return 0;
}
