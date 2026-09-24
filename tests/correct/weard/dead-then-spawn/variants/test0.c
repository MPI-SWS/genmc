/*
 * Each worker is joined before the next one is spawned, so a cut can take
 * a terminator while later threads are still to come. A thread whose
 * terminator was pruned must keep reading as dead.
 */
#include <pthread.h>
#include <stdatomic.h>

#define N 400
#define ROUNDS 4

atomic_int x;

void *worker(void *arg)
{
	for (int i = 0; i < N; i++)
		atomic_fetch_add_explicit(&x, 1, memory_order_acq_rel);
	return NULL;
}

int main(void)
{
	for (int r = 0; r < ROUNDS; r++) {
		pthread_t t;

		pthread_create(&t, NULL, worker, NULL);
		pthread_join(t, NULL);
	}
	return 0;
}
