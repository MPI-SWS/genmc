#define DEFAULT_WRITERS 1

#include <genmc.h>
#include <stdatomic.h>

typedef struct st { int f; } st_t;

_Atomic(st_t *) x[2];
st_t y;

void *thread_writer(void *unused)
{
	atomic_store_explicit(&x[0], &y, memory_order_relaxed);
	atomic_store_explicit(&x[1], &y, memory_order_relaxed);
	return NULL;
}

void *thread_reader(void *unused)
{
	st_t* a;
	int t = 0;
	while (true) {
		for (int i = 0; i < 2; ++i) {
			st_t* b = atomic_load_explicit(&x[i], memory_order_relaxed);
			if (b != NULL) {
				a = b;
			}
		}
		__VERIFIER_assume(a != NULL);
		break;
	}
	return NULL;
}
