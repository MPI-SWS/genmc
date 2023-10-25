#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x = 9;
int a;

void *P0(void *unused)
{
	WRITE_ONCE(a, 1);
	smp_wmb();
	WRITE_ONCE(x, 2);
	return NULL;
}

void *P1(void *unused)
{
	int r1; int r2; int r3;

	r1 = READ_ONCE(x);
	r2 = READ_ONCE(a);
	smp_rmb();
	r3 = READ_ONCE(a);
	return NULL;
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
