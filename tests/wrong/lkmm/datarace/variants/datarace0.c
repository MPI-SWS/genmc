#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;

void *P0(void *unused)
{
	WRITE_ONCE(x, 17);
	return NULL;
}

void *P1(void *unused)
{
	int r1 = x;
	return NULL;
}

int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
