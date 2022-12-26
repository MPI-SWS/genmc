#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <stdatomic.h>

/* Github issue #48: https://github.com/MPI-SWS/genmc/issues/48 */

#define N 2

typedef struct {
	void *data;
} student;


typedef struct {
	uintptr_t key;
} data_t;


atomic_int count = 0;

static inline void *alloc(student **p)
{
	data_t *data = malloc(sizeof(data_t));
	if (p) { /* the if is necessary for the bug to appear*/
		*p = malloc(sizeof(student));
		assert(*p);
	}
	/* relaxed or acquire access is also necessary for the bug to happen */
	atomic_fetch_add_explicit(&count, 1, memory_order_relaxed);
	return data;
}


void *run(void *arg)
{
	int x        = 0;
	student *p   = NULL;
	data_t *data = alloc(&p);
	p->data      = data;
	(void)arg;
	return NULL;
}

int main(void)
{
	pthread_t t[N];

	for (size_t i = 0; i < N; i++) {
		pthread_create(&t[i], NULL, run, NULL);
	}

	for (size_t i = 0; i < N; i++) {
		pthread_join(t[i], NULL);
	}

	return 0;
}
