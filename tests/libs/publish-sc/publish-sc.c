typedef int elem_t;

void myinit(elem_t *loc, elem_t val);
void mywrite(elem_t *loc, elem_t val);
elem_t myread(elem_t *loc);

elem_t x;
elem_t flag;

void *thread_writer(void *arg)
{
	mywrite(&x, 42);
	mywrite(&flag, 1);
	return NULL;
}

void *thread_reader(void *arg)
{
	int local = 0;
	int count = 0;

	local = myread(&flag);
	while (local != 1) {
		count++;
		if (count > 40)
			pthread_exit(NULL);
		local = myread(&flag);
	}
	// printf("got it!\n");
	assert(myread(&x) == 42);
	return NULL;
}
