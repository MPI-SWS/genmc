typedef bool elem_t;

void myinit(elem_t *loc, elem_t val);
void mywrite(elem_t *loc, elem_t val);
elem_t myread(elem_t *loc);

elem_t latch1;
elem_t latch2;
elem_t flag1;
elem_t flag2;

void __VERIFIER_assume(int);

void *thread_1(void *unused)
{
	for (;;) {
		__VERIFIER_assume(myread(&latch1));
		/* assert(!myread(&latch1, access_mode) || myread(&flag1)); */

		mywrite(&latch1, false);
		if (myread(&flag1)) {
			mywrite(&flag1, false);
			mywrite(&flag2, true);
			mywrite(&latch2, true);
		}
	}
	return NULL;
}

void *thread_2(void *unused)
{
	for (;;) {
		__VERIFIER_assume(myread(&latch2));
		/* assert(!myread(&latch2) || myread(&flag2)); */

		mywrite(&latch2, false);
		if (myread(&flag2)) {
			mywrite(&flag2, false);
			mywrite(&flag1, true);
			mywrite(&latch1, true);
		}
	}
	return NULL;
}
