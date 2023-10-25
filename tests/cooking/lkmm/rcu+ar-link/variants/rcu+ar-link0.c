#include <stdlib.h>
#include <pthread.h>
#include <lkmm.h>
#include <assert.h>

/*
 * Buggy test case. Demonstrates an legal rcu behavior, but with a long
 * po; ar; pb; prop; po cycle between the RCU-RSCS and the GP. In addition,
 * it shows why the ar* component at the beginning of rcu-link is necessary.
 */

#define BUG_ON(x) assert(!(x))

/* RCU variables */
int x;
int y;

int r_x;
int r_y;

/* helper variables */
int s;
int w;
int z;

int a;
int b;
int c;

int r_s;
int r_a;
int r_b;
int r_c;

void *P0(void *unused)
{
	rcu_read_lock();
	WRITE_ONCE(x, 1);
	r_y = READ_ONCE(y);
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
	r_s = READ_ONCE(s);
	smp_store_release(&w, 1);
	/* smp_mb(); */
	int r = READ_ONCE(z);
	WRITE_ONCE(a, r + 1);
	return NULL;
}

void *P3(void *unused)
{
	r_a = READ_ONCE(a);
	WRITE_ONCE(b, r_a);
	return NULL;
}

void *P4(void *unused)
{
	r_b = READ_ONCE(b);
	smp_store_release(&c, 1);
	return NULL;
}

void *P5(void *unused)
{
	r_c = READ_ONCE(c);
	synchronize_rcu();
	WRITE_ONCE(y, 1);
	return NULL;
}

int main()
{
	pthread_t t0, t1, t2, t3, t4, t5;

	if (pthread_create(&t0, NULL, P0, NULL))
		abort();
	if (pthread_create(&t1, NULL, P1, NULL))
		abort();
	if (pthread_create(&t2, NULL, P2, NULL))
		abort();
	if (pthread_create(&t3, NULL, P3, NULL))
		abort();
	if (pthread_create(&t4, NULL, P4, NULL))
		abort();
	if (pthread_create(&t5, NULL, P5, NULL))
		abort();

	if (pthread_join(t0, NULL))
		abort();
	if (pthread_join(t1, NULL))
		abort();
	if (pthread_join(t2, NULL))
		abort();
	if (pthread_join(t3, NULL))
		abort();
	if (pthread_join(t4, NULL))
		abort();
	if (pthread_join(t5, NULL))
		abort();

	/* BUG_ON(r_y == 1 && r_s == 1 && r_a == 1 && r_b == 1 && r_c == 1); */

	return 0;
}
