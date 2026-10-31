atomic_int x;
atomic_int y;
atomic_int z;

/*
 * GenMC's ARM model is the thesis' strengthened ARMv8: an RMW write inherits its
 * read's orderings, so here the FADD's write is ordered before y = r0 + 1, as the
 * FADD's read is (data). ARMv8 itself orders neither the FADD's write nor r1 = z
 * with y = r0 + 1 and allows r1 = 1 (leaving x = 1); the strengthened model, IMM
 * (an RMW write is ordered before every later write) and RC11/SC (a po U rf cycle)
 * forbid it, so all models see the same executions.
 */
void *thread_1(void *unused)
{
	int r1 = atomic_load_explicit(&z, memory_order_relaxed);
	int r0 = atomic_fetch_add_explicit(&x, r1, memory_order_relaxed);
	atomic_store_explicit(&y, r0 + 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	int r3 = atomic_load_explicit(&y, memory_order_relaxed);
	atomic_store_explicit(&z, r3, memory_order_relaxed);
	return NULL;
}
