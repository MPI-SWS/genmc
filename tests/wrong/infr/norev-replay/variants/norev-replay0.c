#define __CONFIG_GENMC_INODE_DATA_SIZE 64
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

typedef struct {
	uint64_t r;
	uint64_t y;
	uint64_t t;
} T;

T* q;

#define barrier() asm("")
void     STORE(uint64_t *a, uint64_t v) {barrier();__atomic_store_n(a, v, __ATOMIC_SEQ_CST);}
uint64_t LOAD(uint64_t *a) {barrier();return __atomic_load_n(a, __ATOMIC_SEQ_CST);}

void *thread1 (void *args)
{
	uint64_t x[1];
	for (int j = 0; j < 1; j++)
		x[j] = 1UL;

	q->r = x[0];
	__atomic_store_n(&q->y, 0, __ATOMIC_RELAXED);
	return NULL;
}

void *thread2 (void *args)
{
	LOAD(&q->y);
	assert(q->r);
	return NULL;
}

int
main(int argc, char **argv)
{
	q = (T*) malloc(sizeof(T));
	STORE(&q->y, 0);
	STORE(&q->t, 0);
	STORE(&q->y, 0);

	pthread_t t1, t2;
	pthread_create(&t1, NULL, thread1, 0);
	pthread_create(&t2, 0, thread2, 0);
	pthread_join(t2, 0);

	return 0;
}
