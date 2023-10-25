#include <stdlib.h>
#include <pthread.h>
#include <lkmm.h>
#include <assert.h>

/*
 * Buggy test case. Demonstrates an illegal rcu behavior, but with a
 * po; ar; pb; prop; po cycle between the RCU-RSCS and the GP. It also
 * shows why the ar* component in the beginning of rcu-link is necessary
 * (otherwise, the BUG_ON() fires).
 * Finally, to get the correct number of executions for this test case,
 * we need to include rcu-fence in strong-fence (in other words, this
 * test case demonstrates how redefinition in herd affect previously
 * affected relations).
 */

#define BUG_ON(x) assert(!(x))

/* RCU variables */
int x;
int y;

int r_x;
int r_y;

/* helper variables */
int s;
int w[2] = {0, 1};

int r_s;
int r_w;

void *P0(void *unused)
{
	rcu_read_lock();
	WRITE_ONCE(x, 1);
	WRITE_ONCE(y, 1);
	rcu_read_unlock();
	return NULL;
}

void *P1(void *unused)
{
	if (READ_ONCE(x) == 1)
		WRITE_ONCE(s, 1);
	return NULL;
}

void *P2(void *unused)
{
	r_s = READ_ONCE(s); /* ar from x up to this point */
	r_w = READ_ONCE(w[r_s]); /* prop to P3 */
	return NULL;
}

void *P3(void *unused)
{
	WRITE_ONCE(w[1], 2);
	synchronize_rcu();
	r_y = READ_ONCE(y);
	return NULL;
}

int main()
{
	pthread_t t0, t1, t2, t3;

	if (pthread_create(&t0, NULL, P0, NULL))
		abort();
	if (pthread_create(&t1, NULL, P1, NULL))
		abort();
	if (pthread_create(&t2, NULL, P2, NULL))
		abort();
	if (pthread_create(&t3, NULL, P3, NULL))
		abort();

	if (pthread_join(t0, NULL))
		abort();
	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();
	if (pthread_join(t3, NULL))
		abort();

	BUG_ON(r_y == 0 && r_s == 1 && r_w == 1);

	return 0;
}
