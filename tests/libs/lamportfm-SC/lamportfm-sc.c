/* Lamport's fast mutex algorithm 2
 * (https://users.soe.ucsc.edu/~scott/courses/Fall11/221/Papers/Sync/lamport-tocs87.pdf)
 */

#ifndef N
#  warning "N was not defined, assuming 2"
#  define N 2
#endif

void __VERIFIER_assume(intptr_t);
#define await(cond) __VERIFIER_assume(cond)

typedef intptr_t elem_t;
void myinit(elem_t *loc, elem_t val);
void mywrite(elem_t *loc, elem_t val);
elem_t myread(elem_t *loc);

elem_t b[N+1], x, y;

static void lock(intptr_t thread)
{
	while (1) {
		mywrite(&b[thread], true);
		mywrite(&x, thread);
		if (myread(&y) != 0) {
			mywrite(&b[thread], false);
			await(myread(&y) == 0);
			continue;
		}
		mywrite(&y, thread);
		if (myread(&x) != thread) {
			mywrite(&b[thread], false);
			for (intptr_t j = 1; j <= N; j++)
				await(myread(&b[j]) == false);
			if (myread(&y) != thread) {
				await(myread(&y) == 0);
				continue;
			}
		}
		break;
	}
}

static void unlock(intptr_t thread)
{
	mywrite(&y, 0);
	mywrite(&b[thread], false);
}

static void *thread(void *arg)
{
	intptr_t thread = (intptr_t) arg;
	lock(thread);
	unlock(thread);
	return NULL;
}
