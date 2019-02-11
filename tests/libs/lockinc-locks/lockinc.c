typedef int elem_t;

void myinit(elem_t *loc, elem_t val);
elem_t mylock(elem_t *loc);
void myunlock(elem_t *loc, elem_t val);

elem_t l;

void *thread_n(void *unused)
{
	mylock(&l);
	myunlock(&l, 0);
	return NULL;
}
