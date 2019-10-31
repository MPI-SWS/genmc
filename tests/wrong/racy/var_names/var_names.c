struct bar {
	atomic_int b[4];
	atomic_int c;
};

struct foo {
	atomic_int x;
	struct bar y[2];
	int z;
};

void *thread_1(void *arg)
{
	struct foo *shared = (struct foo *) arg;

	/* Just access random fields to check printing... */
	atomic_store_explicit(&shared->x, 42, memory_order_relaxed);
	atomic_store_explicit(&shared->y[0].b[0], 42, memory_order_relaxed);
	atomic_store_explicit(&shared->y[0].b[1], 42, memory_order_relaxed);
	atomic_store_explicit(&shared->y[0].c, 42, memory_order_relaxed);

	/* And here comes the race */
	shared->z = 42;
	return NULL;
}

void *thread_2(void *arg)
{
	struct foo *shared = (struct foo *) arg;

	/* Ditto... */
	atomic_store_explicit(&shared->x, 42, memory_order_relaxed);
	atomic_store_explicit(&shared->y[1].b[2], 42, memory_order_relaxed);
	atomic_store_explicit(&shared->y[1].b[3], 42, memory_order_relaxed);
	atomic_store_explicit(&shared->y[1].c, 42, memory_order_relaxed);

	/* And here comes the race */
	shared->z = 17;
	return NULL;
}
