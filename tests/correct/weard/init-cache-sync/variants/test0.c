/*
 * A cut can prune a release write before its reader runs; the reader must
 * still acquire what the write had released (through the initializer's
 * per-location cache). The noise thread drives repeated cuts.
 */
#include <pthread.h>
#include <stdatomic.h>

int pad[8];
int noise[30];
int x;
atomic_int y;

void *writer(void *arg)
{
	for (int i = 0; i < 8; i++)
		pad[i] = i;
	x = 42;
	atomic_store_explicit(&y, 1, memory_order_release);
	return NULL;
}

void *reader(void *arg)
{
	if (atomic_load_explicit(&y, memory_order_acquire) == 1) {
		int r = x;
		(void)r;
	}
	return NULL;
}

void *nuisance(void *arg)
{
	for (int i = 0; i < 30; i++)
		noise[i] = i;
	return NULL;
}

int main(void)
{
	pthread_t w, r, n;

	pthread_create(&w, NULL, writer, NULL);
	pthread_create(&r, NULL, reader, NULL);
	pthread_create(&n, NULL, nuisance, NULL);

	pthread_join(w, NULL);
	pthread_join(r, NULL);
	pthread_join(n, NULL);
	return 0;
}
