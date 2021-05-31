#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

int x;
int y;

void *P0(void *unused)
{
	int r1; int r3;

	r1 = READ_ONCE(x);
	r3 = (r1 != 0);
	if (r3) {
		WRITE_ONCE(y, 1);
	}
	return NULL;
}

void *P1(void *unused)
{
	int r2; int r4;

	r2 = READ_ONCE(y);
	r4 = (r2 != 0);
	if (r4) {
		WRITE_ONCE(x, 1);
	}
	return NULL;
}

void *P2(void *unused)
{
	WRITE_ONCE(x, 2);
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
