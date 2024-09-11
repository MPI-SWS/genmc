#ifndef MAX
# define MAX 42
#endif

atomic_int AR[MAX];
atomic_int back;

int r_1, r_2, r_3, r_4;

void enqueue(int a)
{
	int k = atomic_fetch_add_explicit(&back, 1, memory_order_acq_rel);
	atomic_store_explicit(&AR[k], a, memory_order_release);
	return;
}

int dequeue(int expected)
{
	int lback = atomic_load_explicit(&back, memory_order_acquire);
	int lan, k;

	for (lan = k = 0; lan == 0; ++k) {
		__VERIFIER_assume(k < lback);
		lan = atomic_exchange_explicit(&AR[k], 0, memory_order_acq_rel);
		// __VERIFIER_assume(lan == expected || lan == 0);
	}
	return lan;
}

void *thread_1(void *unused)
{
	enqueue(1);
	r_2 = dequeue(2);
	return NULL;
}

void *thread_2(void *unused)
{
	enqueue(2);
	enqueue(3);
	return NULL;
}

void *thread_3(void *unused)
{
	r_3 = dequeue(3);
	enqueue(4);
	return NULL;
}

void *thread_4(void *unused)
{
	r_4 = dequeue(4);
	r_1 = dequeue(1);
	return NULL;
}
