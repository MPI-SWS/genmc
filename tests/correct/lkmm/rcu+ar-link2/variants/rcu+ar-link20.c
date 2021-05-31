#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;
int a;
int s;

int r0;
int r2;
int r6;

void *P0(void *unused)
{
	rcu_read_lock();
	WRITE_ONCE(x, 2);
	r0 = READ_ONCE(y);
	rcu_read_unlock();
	return NULL;
}

void *P1(void *unused)
{
	WRITE_ONCE(x, 1);
	smp_mb();
	WRITE_ONCE(s, 1);
	return NULL;
}

void *P2(void *unused)
{
	int r = READ_ONCE(s);
	if (r) {
		r2 = r;
		WRITE_ONCE(a, 2);
	}
	return NULL;
}

void *P3(void *unused)
{
	WRITE_ONCE(a, 1);
	synchronize_rcu();
	WRITE_ONCE(y, 1);
	return NULL;
}

int main()
{
	pthread_t t0, t1, t2, t3;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);
	pthread_create(&t3, NULL, P3, NULL);

	pthread_join(t0, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	assert(!(r0 == 1 && r2 == 1 && a == 1 && x == 1));

	return 0;
}
