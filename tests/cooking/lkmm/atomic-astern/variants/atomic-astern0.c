#include <stdlib.h>
#include <lkmm.h>
#include <pthread.h>
#include <assert.h>

atomic_t x = ATOMIC_INIT(2);

void *P0(void *unused)
{
        int r1;

        r1 = atomic_dec_and_test(&x);
	return NULL;
}

void *P1(void *unused)
{
        int r2;

        r2 = atomic_dec_and_test(&x);
	return NULL;
}


int main()
{
	pthread_t t0, t1;

	pthread_create(&t0, NULL, P0, NULL);
	pthread_create(&t1, NULL, P1, NULL);

	return 0;
}
