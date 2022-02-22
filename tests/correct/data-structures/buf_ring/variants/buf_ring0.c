#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <genmc.h>

#include "../buf_ring.c"

int main() {
	pthread_t t[NTHREADS];
	br = buf_ring_alloc(BUF_SIZE);
	for (intptr_t i = 0; i < NTHREADS; i++) pthread_create(t+i, 0, run, (void*)i);
	for (intptr_t i = 0; i < NTHREADS; i++) pthread_join(t[i], 0);
	return 0;
}
