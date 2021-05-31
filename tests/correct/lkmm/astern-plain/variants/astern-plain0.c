#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

int x;
int y;

void *P0(void *unused)
{
	WRITE_ONCE(x, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r1 = READ_ONCE(x);
	__VERIFIER_assume(r1 == 1);

	WRITE_ONCE(x, 2);
	smp_store_release(&y, 1);
	return NULL;
}

void *P2(void *unused)
{
	int r1 = smp_load_acquire(&y);
	if (r1)
		x = 3;
	return NULL;
}

int main()
{
	pthread_t t0, t1, t2;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);
	pthread_create(&t2, NULL, P2, NULL);

	return 0;
}
