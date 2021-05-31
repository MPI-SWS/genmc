#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int y;
int x;

void *P0(void *unused)
{
	x = 1;
	smp_store_release(&y, 1);
	return NULL;
}

void *P1(void *unused)
{
	int r0;
	int r1 = -1;

	r0 = smp_load_acquire(&y);
	if (r0)
		r1 = x;
	return NULL;
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
