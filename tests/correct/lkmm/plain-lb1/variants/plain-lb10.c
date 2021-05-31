#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int a;
int b;

int *x = &a;
int y;

/* Result: Never */

void *P0(void *unused)
{
	int *r0;

	r0 = rcu_dereference(x);	/* A */
	*r0 = 0;	/* C */
	smp_wmb();
	WRITE_ONCE(y, 1);	/* D */
	return NULL;
}

void *P1(void *unused)
{
	int r0;

	r0 = READ_ONCE(y);
	rcu_assign_pointer(x, &b);
	return NULL;
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
