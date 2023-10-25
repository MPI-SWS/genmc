#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/* Result: Never */

int a;

int y;
int *x = &a;

void *P0(void *unused)
{
	int *r0 = rcu_dereference(x);
	*r0 = 2;
	return NULL;
}

void *P1(void *unused)
{
	WRITE_ONCE(y, 1);
	rcu_assign_pointer(x, &y);
	return NULL;
}

/* exists (0:r0=y /\ (~y=2 \/ a=2)) */

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
