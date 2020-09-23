/*
 * Thread-Local-Storage Test 3
 *
 * Thread 1 performs an unconditional load of 'x', and writes the value
 * value 17 to thread-local variable 'v', if the value read is 1.
 * Thread 2 writes 1 to 'x' (which will cause the revisit of the load
 * of Thread 1), and then reads 'v'. If the value read is 17, Thread 2
 * writes 42 to 'x'. Thread 2 should not be able to read the value 17 for 'v'.
 */

int __thread v;
atomic_int x;

void *thread_1(void *unused)
{
	if (atomic_load(&x) == 1)
		v = 17;
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_store(&x, 1);
	if (v == 17)
		atomic_store(&x, 42);
	return NULL;
}
