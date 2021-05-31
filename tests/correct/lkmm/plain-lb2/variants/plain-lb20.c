#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

/*
 * Result: Sometimes
 *
 * Possible write-to-read optimization of P0() plain write to *r0
 * can render the smp_wmb() ineffective.  (Assuming we choose to
 * model the write-to-read optimization in this case.)
 */

int a;
int b;

int y;
int *x = &a;

void *P0(void *unused)
{
	int *r0;

	r0 = rcu_dereference(x);	/* A */
	if (*r0)	/* B */
		*r0 = 0;	/* C */
	smp_wmb();
	WRITE_ONCE(y, 1);	/* D */
	return NULL;
}

void *P1(void *unused)
{
	int r0 = READ_ONCE(y);
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
