#include <stdlib.h>
#include <pthread.h>
#include <lkmm.h>
#include <assert.h>

int x;
int y;

int r_x;
int r_y;

void *P0(void *unused)
{
	rcu_read_lock();
	r_x = READ_ONCE(x);
	r_y = READ_ONCE(y);
	rcu_read_unlock();
	return NULL;
}

void *P1(void *unused)
{
	WRITE_ONCE(x, 1);
	synchronize_rcu();
	WRITE_ONCE(y, 1);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, P0, NULL))
		abort();
	if (pthread_create(&t2, NULL, P1, NULL))
		abort();

	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();

	assert(!(r_x == 0 && r_y == 1));

	return 0;
}
