#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int a;
int c;
int d;

void *P0(void *unused)
{
	rcu_read_lock();
	int r1 = READ_ONCE(x);
	WRITE_ONCE(a, 1);
	rcu_read_unlock();
	return NULL;
}

void *P1(void *unused)
{
	int r2 = READ_ONCE(a);
	synchronize_rcu();
	WRITE_ONCE(c, 1);
	return NULL;
}

void *P2(void *unused)
{
	int r3 = READ_ONCE(c);
	synchronize_rcu();
	WRITE_ONCE(d, 1);
	return NULL;
}

void *P3(void *unused)
{
	rcu_read_lock();
	int r4 = READ_ONCE(d);
	WRITE_ONCE(x, 1);
	rcu_read_unlock();
	return NULL;
}

int main()
{
	pthread_t t0, t1, t2, t3;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);
	pthread_create(&t3, NULL, P3, NULL);

	return 0;
}
