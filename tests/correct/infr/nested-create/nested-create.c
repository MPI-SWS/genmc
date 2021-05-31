/*
 * Exercises nested thread creation. Ensures that new threads properly
 * synchronize with their parent create statement, even if this parent
 * create can spawn threads with different hb-predecessors each time.
 */

atomic_int x;
atomic_int y;
atomic_int z;
atomic_int w;

atomic_int flag;

void *thread_0(void *unused)
{
	flag = 1;
	return NULL;
}


void *thread_1(void *unused)
{
	z = 42;
	x = 42;
	return NULL;
}

void *thread_n(void *arg)
{
	if (arg == (void *) 1)
		assert(z == 42);
	else
		assert(w == 42);
	return NULL;
}

void *thread_2(void *unused)
{
	pthread_t tn;

	if (flag == 0) {
		__VERIFIER_assume(x == 42);
		pthread_create(&tn, NULL, thread_n, (void *) 1);
	} else {
		int r = y;
		__VERIFIER_assume(y == 42);
		pthread_create(&tn, NULL, thread_n, (void *) 2);
	}
	return NULL;
}

void *thread_3(void *unused)
{
	w = 42;
	y = 42;
	return NULL;
}
