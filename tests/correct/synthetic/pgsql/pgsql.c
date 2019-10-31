#if defined(NIDHUGG)
#define smp_mb() asm volatile ("mfence" ::: "memory")
#elif defined(NIDHUGG_POWER)
#define smp_mb() asm volatile("sync" ::: "memory")
#else
#define smp_mb() atomic_thread_fence(memory_order_seq_cst)
#endif

#ifdef MAKE_ALL_SC
# define access_mode memory_order_seq_cst
#else
# define access_mode memory_order_relaxed
#endif

atomic_bool latch1 = ATOMIC_VAR_INIT(true);
atomic_bool latch2 = ATOMIC_VAR_INIT(false);
atomic_bool flag1  = ATOMIC_VAR_INIT(true);
atomic_bool flag2  = ATOMIC_VAR_INIT(false);

void __VERIFIER_assume(int);

void *thread_1(void *unused)
{
	for (;;) {
		__VERIFIER_assume(atomic_load_explicit(&latch1, access_mode));
		/* assert(!atomic_load_explicit(&latch1, access_mode) || */
		/*        atomic_load_explicit(&flag1, access_mode)); */

		atomic_store_explicit(&latch1, false, access_mode);
		if (atomic_load_explicit(&flag1, access_mode)) {
			atomic_store_explicit(&flag1, false, access_mode);
			atomic_store_explicit(&flag2, true, access_mode);
			smp_mb();
			atomic_store_explicit(&latch2, true, access_mode);
		}
	}
	return NULL;
}

void *thread_2(void *unused)
{
	for (;;) {
		__VERIFIER_assume(atomic_load_explicit(&latch2, access_mode));
		/* assert(!atomic_load_explicit(&latch2, access_mode) || */
		/*        atomic_load_explicit(&flag2, access_mode)); */

		atomic_store_explicit(&latch2, false, access_mode);
		if (atomic_load_explicit(&flag2, access_mode)) {
			atomic_store_explicit(&flag2, false, access_mode);
			atomic_store_explicit(&flag1, true, access_mode);
			smp_mb();
			atomic_store_explicit(&latch1, true, access_mode);
		}
	}
	return NULL;
}
