#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/* Result: Never DATARACE */

int a;

int y;
int *x = &a;

void *P0(void *unused)
{
	int *r0 = rcu_dereference(x);
	int r1 = *r0;
	return NULL;
}

void *P1(void *unused)
{
	int r0 = READ_ONCE(y);
	rcu_assign_pointer(x, &y);
	return NULL;
}

void *P2(void *unused)
{
	WRITE_ONCE(y, 1);
	return NULL;
}

/* exists (0:r0=y /\ 0:r1=0 /\ 1:r0=1) */

int main()
{
	pthread_t t0, t1, t2;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	return 0;
}
