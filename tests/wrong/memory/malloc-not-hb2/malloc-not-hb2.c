// also for acq
#define mo_rlx memory_order_relaxed

_Atomic(int *) p;

void *thread1(void *unused)
{
	p = malloc(sizeof(int));
	//*p = 17;
	/* free(p); */
	return NULL;
}

void *thread2(void *unused)
{
	int *r = atomic_load_explicit(&p, mo_rlx);
	if (r) {
		*r = 18;
	}
	return NULL;
}
