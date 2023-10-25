#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int a;
int b;

void *P0(void *unused)
{
	WRITE_ONCE(a, 1);
	synchronize_rcu();
	int r1 = READ_ONCE(b);
	return NULL;
}

void *P1(void *unused)
{
	rcu_read_lock();
	int r1 = READ_ONCE(a);
	if (r1 == 0) {
		WRITE_ONCE(b, 1);
		WRITE_ONCE(b, 0);
	}
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
