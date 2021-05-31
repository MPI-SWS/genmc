#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int a;
int c;
int d;

void *P0(void *unused)
{
	WRITE_ONCE(a, 1);
	WRITE_ONCE(x, 2);
	smp_wmb();
	WRITE_ONCE(c, 3);
	WRITE_ONCE(d, 4);
	return NULL;
}

void *P1(void *unused)
{
	int r1; int r2; int r3; int r4;

	r1 = READ_ONCE(c);
	r2 = READ_ONCE(d);
	smp_rmb();
	r3 = READ_ONCE(a);
	r4 = READ_ONCE(x);
	return NULL;
}

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
