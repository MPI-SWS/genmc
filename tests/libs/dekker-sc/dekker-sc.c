typedef int elem_t;

void myinit(elem_t *loc, elem_t val);
void mywrite(elem_t *loc, elem_t val);
elem_t myread(elem_t *loc);

elem_t x;
elem_t y;
elem_t z1;
elem_t z2;

void *thread_1(void *unused)
{
	mywrite(&y, 1);
	if (!myread(&x))
		mywrite(&z1, 1);
	return NULL;
}

void *thread_2(void *unused)
{
	mywrite(&x, 1);
	if (!myread(&y))
		mywrite(&z2, 1);
	return NULL;
}

void *thread_3(void *unused)
{
	if (myread(&z1) && myread(&z2)) {
		for (int i = 0; i < N; i++)
			mywrite(&x, i);
	}
	return NULL;
}

void *thread_4(void *unused)
{
	if (myread(&z1) && myread(&z2)) {
		for (int i = 0; i < N; i++)
			myread(&x);
	}
	return NULL;
}
