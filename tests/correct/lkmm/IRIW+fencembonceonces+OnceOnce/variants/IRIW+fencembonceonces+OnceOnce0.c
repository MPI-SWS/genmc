#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;

void *P0(void *unused)
{
	WRITE_ONCE(x, 1);
}

void *P1(void *unused)
{
	int r0;
	int r1;

	r0 = READ_ONCE(x);
	smp_mb();
	r1 = READ_ONCE(y);
}

void *P2(void *unused)
{
	WRITE_ONCE(y, 1);
}

void *P3(void *unused)
{
	int r0;
	int r1;

	r0 = READ_ONCE(y);
	smp_mb();
	r1 = READ_ONCE(x);
}

int main()
{
	pthread_t t0, t1, t2, t3;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);
	pthread_create(&t3, NULL, P3, NULL);

	return 0;
}
