atomic_int x;
atomic_int y;

void *thread_1(void *arg)
{
	atomic_store_explicit(&x, 1, memory_order_seq_cst);
	atomic_load_explicit(&y, memory_order_seq_cst);
	return NULL;
}

void *thread_2(void *arg)
{
	atomic_store_explicit(&y, 1, memory_order_seq_cst);
	atomic_load_explicit(&x, memory_order_seq_cst);
	return NULL;
}
