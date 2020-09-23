/*
 * Thread-Local-Storage Test 1
 *
 * This testcase checks whether different threads write to the same
 * memory location w.r.t. a thread-local variable. To examine this,
 * we can use a simple SB testcase and assert that each thread
 * reads the value it just wrote, and the value the other thread
 * wrote. The fact that there are revisitable reads ensures that
 * the loads for the thread-local variable read the expected value
 * even in recursive explorations. This testcase also ensures that
 * accesses to a thread-local variable do not constitute a race.
 */

atomic_int x;
atomic_int y;

int __thread cpu;

void *thread_1(void *unused)
{
	atomic_store(&x, 42);
	atomic_load(&y);
	cpu = 1;
	assert(cpu == 1);
	return NULL;
}

void *thread_2(void *unused)
{

	atomic_store(&y, 42);
	atomic_load(&x);
	cpu = 42;
	assert(cpu == 42);
	return NULL;
}
