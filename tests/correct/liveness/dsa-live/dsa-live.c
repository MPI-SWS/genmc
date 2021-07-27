/* Test case: Diogo Behrens (Github #17) */

int x;
atomic_int lock = 1;

void *run_t0(void *arg)
{
	/* initial lock owner */
	x = 1; // critical section

	/* release lock (SC) */
	lock = 0;
	return NULL;
}

static void *run_t1(void *arg)
{
	/* acquire lock */
	do {
		while (lock != 0);
	} while (atomic_exchange(&lock, 1));

	/* because atomic_exchange is acquire, and lock = 0 is release, the
	 * following assertion must be true */
	assert(x == 1 && "unexpected value");

	return NULL;
}
