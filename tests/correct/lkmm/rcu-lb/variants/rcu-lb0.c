#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;

void *P0(void *unused)
{
	rcu_read_lock();
	int r1 = READ_ONCE(x);
	rcu_read_unlock();

	rcu_read_lock();
	WRITE_ONCE(y, 1);
	rcu_read_unlock();
	return NULL;
}

void *P1(void *unused)
{
	rcu_read_lock();
	int r2 = READ_ONCE(y);
	rcu_read_unlock();

	rcu_read_lock();
	WRITE_ONCE(x, 1);
	rcu_read_unlock();
	return NULL;
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
