#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

atomic_int data;

#define N_ITER 2

void *thread(void *unused)
{
#ifdef USE_DEC_WHILE1
	int lmt = N_ITER;
	while (lmt--) data++;

	// error

#elif USE_DEC_WHILE2
	int lmt = N_ITER + 1;
	while (--lmt) data++;

	// error

#elif USE_LONG_WHILE
	int lmt = N_ITER;
	while (lmt) {
		data++;
		lmt--;
	}

	// ok

#elif USE_INC_WHILE
	int lmt = 0;
	while (lmt++ < N_ITER) data++;

	// error

#elif USE_FOR
	for (int i = 0; i < N_ITER; i++) data++;

	// ok

#elif USE_IF
	int lmt = N_ITER;
	while (1) {
		if (lmt--) data++;
		else break;
	}

	// error

#else
	DO NOT COMPILE -- missing variant
#endif

	return NULL;
}

int main()
{
	pthread_t t;
	data = 0;

	pthread_create(&t, NULL, thread, NULL);

	pthread_join(t, NULL);

	assert(data == N_ITER);
}
