#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/* List: x->y then x->z. Noncircular, but long enough. */
int *z = (int *) 1;
int *y = (int *) 2;
int *x;

void *P0(void *unused)
{
	rcu_assign_pointer(x, z); /* Remove y from list. */
	synchronize_rcu();
	WRITE_ONCE(y, 0); /* Emulate kfree(y). */
	return NULL;
}

void *P1(void *unused)
{
	rcu_read_lock();
	int *r1 = rcu_dereference(x); /* Pick up list head. */
	int *r2 = rcu_dereference(r1); /* Pick up value. */
	rcu_read_unlock();
	return NULL;
}

int main()
{
	pthread_t t0, t1;

	WRITE_ONCE(x, y);

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
