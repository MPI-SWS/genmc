#include <stdlib.h>
#include <pthread.h>
#include <lkmm.h>
#include <assert.h>

/*
 * Buggy test case. Exercises the interactions of WB with the rest of
 * LKMM.  It also shows why relations like rcu-link and rcu-fence
 * cannot be stored in local matrices in RCUCalculator, but rather
 * they have to be stored in the EG.

 * More specifically, in order to find the bug in this test case, we
 * have to locate some extension of WB that does not render the
 * execution invalid. The first one tried by genmc is invalid, and if
 * we do not restore rcu-link and rcu-fence to their earlier versions
 * (that did not take into account the extended WB), we will not be
 * able to find the bug. To do that restoration, however, we have to
 * store them along with all other matrices in the EG.
 */

#define BUG_ON(x) assert(!(x))

int x;
int y;
int z;

void *P0(void *unused)
{
	rcu_read_lock();
	int r0 = READ_ONCE(x);
	WRITE_ONCE(y, 1); /* Writes to y are unordered by WB */
	rcu_read_unlock();

	rcu_read_lock();
	BUG_ON(READ_ONCE(z) == 1 && READ_ONCE(x) == 1 && READ_ONCE(y) == 1);
	rcu_read_unlock();
	return NULL;
}

void *P1(void *unused)
{
	WRITE_ONCE(x, 1);
	synchronize_rcu();
	WRITE_ONCE(y, 2);
	synchronize_rcu();
	WRITE_ONCE(z, 1);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	if (pthread_create(&t1, NULL, P0, NULL))
		abort();
	if (pthread_create(&t2, NULL, P1, NULL))
		abort();

	return 0;
}
