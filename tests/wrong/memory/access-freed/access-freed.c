#define mo_acq memory_order_acquire

_Atomic(int *) p;

void *thread1(void *unused)
{
	p = malloc(sizeof(int));
	free(p);
	return NULL;
}

void *thread2(void *unused)
{
	int *r = atomic_load_explicit(&p, mo_acq);
	if (r) {
		*r = 18;
	}
	return NULL;
}
