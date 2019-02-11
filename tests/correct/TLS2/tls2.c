/*
 * Thread-Local-Storage Test 2
 *
 * This testcase ensures that no data races occur between
 * thread-local variables.
 */

atomic_int __thread x;

void *thread_1(void *unused)
{
	atomic_store(&x, 42);
	return NULL;
}

void *thread_2(void *unused)
{
	atomic_load(&x);
	return NULL;
}

void *thread_3(void *unused)
{
	atomic_store(&x, 42);
	return NULL;
}
