#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;

void *P0(void *unused)
{
	int r1;
	int r2;

	WRITE_ONCE(x, 1);
	r1 = READ_ONCE(x);
	r2 = READ_ONCE(y);
	return NULL;
}

void *P1(void *unused)
{
	int r3;
	int r4;

	WRITE_ONCE(y, 1);
	r3 = READ_ONCE(y);
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
