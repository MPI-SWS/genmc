#include <genmc.h>

atomic_int x;
atomic_int y;

void *thread_one(void *arg)
{
	atomic_store_explicit(&x, 1, memory_order_release);
	atomic_load_explicit(&y, memory_order_acquire);
	return NULL;
}

void *thread_two(void *arg)
{
	atomic_store_explicit(&y, 1, memory_order_release);
	atomic_load_explicit(&x, memory_order_acquire);
	return NULL;
}
