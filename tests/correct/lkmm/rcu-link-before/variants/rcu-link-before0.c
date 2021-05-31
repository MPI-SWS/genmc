#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int a;
int b;
int c;

void *P0(void *unused)
{
	WRITE_ONCE(a, 1);
	return NULL;
}

void *P1(void *unused)
{
	rcu_read_lock();
	WRITE_ONCE(a, 2);
	WRITE_ONCE(b, 1);
	rcu_read_unlock();
	return NULL;
}

void *P2(void *unused)
{
	WRITE_ONCE(b, 2);
	synchronize_rcu();
	WRITE_ONCE(c, 1);
	return NULL;
}

void *P3(void *unused)
{
	int r1;
	int r2;

	r1 = READ_ONCE(c);
	if (r1)
		r2 = READ_ONCE(a);
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
