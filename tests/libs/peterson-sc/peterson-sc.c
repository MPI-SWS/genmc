typedef int elem_t;

void myinit(elem_t *loc, elem_t val);
void mywrite(elem_t *loc, elem_t val);
elem_t myread(elem_t *loc);

elem_t flag1;  /* Boolean flags */
elem_t flag2;

elem_t turn;   /* Atomic integer that holds the ID of the thread whose turn it is */
elem_t x;      /* Boolean variable to test mutual exclusion */

void __VERIFIER_assume(int);

void *thread_1(void *arg)
{
	mywrite(&flag1, 1);
	mywrite(&turn, 1);

	__VERIFIER_assume(myread(&flag2) != 1 || myread(&turn) != 1);

	/* critical section beginning */
	mywrite(&x, 0);
//	assert(atomic_load_explicit(&x, memory_order_acquire) <= 0);
	myread(&x);
	/* critical section ending */

	mywrite(&flag1, 0);
	return NULL;
}

void *thread_2(void *arg)
{
	mywrite(&flag2, 1);
	mywrite(&turn, 0);

	__VERIFIER_assume(myread(&flag1) != 1 || myread(&turn) != 0);

	/* critical section beginning */
	mywrite(&x, 1);
//	assert(atomic_load_explicit(&x, memory_order_acquire) >= 1);
	myread(&x);
	/* critical section ending */

	mywrite(&flag2, 0);
	return NULL;
}
