atomic_int x = ATOMIC_VAR_INIT(42);

void *thread_1(void *unused)
{
	for (;;) {
		atomic_fetch_sub_explicit(&x, 1, memory_order_relaxed);
		atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
		__VERIFIER_assume(atomic_load_explicit(&x, memory_order_relaxed) == 42);
	}
	return NULL;
}

void *thread_2(void *unused)
{
	for (;;) {
		atomic_fetch_add_explicit(&x, -1, memory_order_relaxed);
		atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
		__VERIFIER_assume(atomic_load_explicit(&x, memory_order_relaxed) == 42);
	}
	return NULL;
}
