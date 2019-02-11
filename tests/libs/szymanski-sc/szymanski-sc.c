typedef int elem_t;

void myinit(elem_t *loc, elem_t val);
void mywrite(elem_t *loc, elem_t val);
elem_t myread(elem_t *loc);

elem_t x;
elem_t flag1;
elem_t flag2;

void __VERIFIER_assume(int);

void *thread_1(void *unused)
{
	mywrite(&flag1, 1);
	__VERIFIER_assume(myread(&flag2) < 3);

	mywrite(&flag1, 3);
	if (myread(&flag2) == 1) {
		mywrite(&flag1, 2);
		__VERIFIER_assume(myread(&flag2) == 4);
	}

	mywrite(&flag1, 4);
	__VERIFIER_assume(myread(&flag2) < 2);

	/* Critical section start */
	mywrite(&x, 0);
	myread(&x);
	assert(myread(&x) <= 0);
	/* Critical section end */

	__VERIFIER_assume(2 > myread(&flag2) || myread(&flag2) > 3);
	mywrite(&flag1, 0);
	return NULL;
}

void *thread_2(void *unused)
{
	mywrite(&flag2, 1);
	__VERIFIER_assume(myread(&flag1) < 3);

	mywrite(&flag2, 3);
	if (myread(&flag1) == 1) {
		mywrite(&flag2, 2);
		__VERIFIER_assume(myread(&flag1) == 4);
	}

	mywrite(&flag2, 4);
	__VERIFIER_assume(myread(&flag1) < 2);

	/* Critical section start */
	mywrite(&x, 1);
	myread(&x);
	assert(myread(&x) >= 1);
	/* Critical section end */

	__VERIFIER_assume(2 > myread(&flag1) || myread(&flag1) > 3);
	mywrite(&flag2, 0);
	return NULL;
}
