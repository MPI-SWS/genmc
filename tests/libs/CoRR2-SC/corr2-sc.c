typedef int elem_t;

void myinit(elem_t *loc, elem_t val);
void mywrite(elem_t *loc, elem_t val);
elem_t myread(elem_t *loc);

elem_t x;

void *thread_1(void *unused)
{
	mywrite(&x, 1);
	return NULL;
}

void *thread_2(void *arg)
{
	mywrite(&x, 2);
	return NULL;
}

void *thread_3(void *arg)
{
	myread(&x);
	myread(&x);
	return NULL;
}

void *thread_4(void *arg)
{

	myread(&x);
	myread(&x);
	return NULL;
}
