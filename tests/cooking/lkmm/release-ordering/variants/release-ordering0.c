#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int u;
int x;
int y;
int z;

void *P0(void *unused)
{
	int r1;
	int r2;

	r1 = READ_ONCE(z);
	smp_store_release(&u, 1);
	r2 = READ_ONCE(x);
	WRITE_ONCE(y, r2);
	return NULL;
}

void *P1(void *unused)
{
	WRITE_ONCE(y, 3);
	smp_wmb();
	WRITE_ONCE(z, 1);
	return NULL;
}

void *P2(void *unused)
{
	int r3;

	r3 = READ_ONCE(u);
	smp_mb();
	WRITE_ONCE(x, 1);
	return NULL;
}

void *P3(void *unused)
{
	WRITE_ONCE(x, 2);
	return NULL;
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
