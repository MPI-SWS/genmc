#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "../fib_bench1.c"

int main()
{
	pthread_t id1, id2;

	if (pthread_create(&id1, NULL, t1, NULL))
		abort();
	if (pthread_create(&id2, NULL, t2, NULL))
		abort();

	__VERIFIER_atomic_begin();
	int condI = i > 144;
	__VERIFIER_atomic_end();

	__VERIFIER_atomic_begin();
	int condJ = j > 144;
	__VERIFIER_atomic_end();

	if (condI || condJ) {
	ERROR: __VERIFIER_error();
	}

	return 0;
}
