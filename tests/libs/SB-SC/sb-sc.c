typedef int elem_t;

elem_t myread(elem_t *loc);
void mywrite(elem_t *loc, elem_t val);
void myinit(elem_t *loc, elem_t val);

elem_t x;
elem_t y;

void *thread_1(void *unused)
{
	mywrite(&x, 1);
	elem_t a = myread(&y);
	return NULL;
}

void *thread_2(void *unused)
{
	mywrite(&y, 1);
	elem_t b = myread(&x);
	return NULL;
}
