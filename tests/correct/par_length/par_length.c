#ifndef N
#  warning "N was not defined, assuming 2"
#  define N 2
#endif

#define T 2

atomic_int v;
atomic_int local[T];

void *thread_n(void *arg)
{
	intptr_t tid = (intptr_t) arg;

	for (int i = 0; i < N; ++i) {
		atomic_store(local + tid, i);
		atomic_load(local + tid);
	}

	atomic_store(&v, tid);
	atomic_load(&v);
	return NULL;
}
