/*
 * An example demonstrating why we should treat assume() statements like
 * if statements, in the sense that the former also create control dependencies.
 *
 * For this example, if we do not record control dependencies when an assume
 * is encountered, sometimes we miss the following execution:
 *
 *                           Ry3  |  Rx1
 *                           Ry4  |  Wy3
 *                           Wx1  |  Fsc
 *                                |  Wy4
 *
 * The reason for that is that the Wx1 does not appear until the assume()
 * before it is true which, in turn, does not happen until both reads
 * get revisited. However, when the second read gets revisited, the Rx
 * is part of the prefix of Wy4, which makes it non-revisitable, and hence
 * unable to read from Wx1.
 *
 * In the case where we randomize the schedule, we get the above execution
 * since it can happen that the second Ry is added after Wy4 (after the
 * first Ry is revisited by Wy3), thus keeping Rx in a revisitable state.
 *
 * The solution for this problem is to *never* get the above
 * execution, by introducing dependencies via the assume().
 */

atomic_int x;
atomic_int y;

void __VERIFIER_assume(int);

void *thread_1(void *unused)
{
	__VERIFIER_assume(2 > atomic_load_explicit(&y, memory_order_relaxed) ||
			  atomic_load_explicit(&y, memory_order_relaxed) > 3);
	atomic_store_explicit(&x, 1, memory_order_relaxed);
	return NULL;
}

void *thread_2(void *unused)
{
	__VERIFIER_assume(atomic_load_explicit(&x, memory_order_relaxed) < 3);

	atomic_store_explicit(&y, 3, memory_order_relaxed);
	atomic_thread_fence(memory_order_seq_cst);
	atomic_store_explicit(&y, 4, memory_order_relaxed);
	return NULL;
}
