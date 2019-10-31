atomic_int i = 1;
atomic_int j = 1;

#define NUM 5

void *t1(void* arg)
{
	for (int k = 0; k < NUM; k++) {
		int prevI = atomic_load_explicit(&i, memory_order_relaxed);
		int prevJ = atomic_load_explicit(&j, memory_order_relaxed);
		atomic_store_explicit(&i, prevI + prevJ, memory_order_relaxed);
	}
	return NULL;
}

void *t2(void* arg)
{
	for (int k = 0; k < NUM; k++) {
		int prevI = atomic_load_explicit(&i, memory_order_relaxed);
		int prevJ = atomic_load_explicit(&j, memory_order_relaxed);
		atomic_store_explicit(&j, prevI + prevJ, memory_order_relaxed);
	}
	return NULL;
}

void *t3(void *arg)
{
	if (atomic_load_explicit(&i, memory_order_relaxed) >= 144 ||
	    atomic_load_explicit(&j, memory_order_relaxed) >= 144)
		assert(0);
	return NULL;
}
