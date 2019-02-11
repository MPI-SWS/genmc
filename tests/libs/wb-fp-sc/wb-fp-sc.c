typedef int elem_t;

void myinit(elem_t *loc, elem_t val);
void mywrite(elem_t *loc, elem_t val);
elem_t myread(elem_t *loc);

elem_t a, b, c, d;

void __VERIFIER_assume(int);

void *thread_1(void *unused)
{
	mywrite(&c, 2);
	mywrite(&a, 2);
	__VERIFIER_assume(myread(&b) == 2);
	return NULL;
}

void *thread_2(void *unused)
{
	mywrite(&c, 1);
	__VERIFIER_assume(myread(&a) == 2);
	__VERIFIER_assume(myread(&b) == 1);
	return NULL;
}

void *thread_3(void *unused)
{
	mywrite(&b, 2);
	mywrite(&a, 1);
	__VERIFIER_assume(myread(&c) == 2);;
	return NULL;
}

void *thread_4(void *unused)
{
	mywrite(&b, 1);
	__VERIFIER_assume(myread(&a) == 1);
	__VERIFIER_assume(myread(&c) == 1);
	return NULL;
}
