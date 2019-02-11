typedef int elem_t;

void myinit(elem_t *loc, elem_t val);
void mywrite(elem_t *loc, elem_t val);
elem_t myread(elem_t *loc);

elem_t array[N+1];
int idx[N+1];

void *thread_reader(void *unused)
{
	for (int i = N; myread(&array[i]) != 0; i--)
		;
	return NULL;
}

void *thread_writer(void *arg)
{
	int j = *((int *) arg);

	mywrite(&array[j], myread(&array[j - 1]) + 1);
	return NULL;
}
