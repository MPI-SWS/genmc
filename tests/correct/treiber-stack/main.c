#include "my_stack.c"

#define NUM_THREADS 4

static mystack_t stack;
static int num_threads = NUM_THREADS;
static pthread_t threads[NUM_THREADS];
static int param[NUM_THREADS];

unsigned int idx1, idx2;
unsigned int a, b;

atomic_int x[3];

int get_thread_num()
{
	pthread_t curr = pthread_self();
	for (int i = 0; i < num_threads; i++)
		if (curr == threads[i])
			return i;
	assert(0);
	return -1;
}

void *main_task(void *param)
{
	unsigned int val;
	int pid = *((int *)param);

	if (pid % 4 == 0) {
		atomic_store_explicit(&x[1], 17, relaxed);
		push(&stack, 1);
	} else if (pid % 4 == 1) {
		atomic_store_explicit(&x[2], 37, relaxed);
		push(&stack, 2);
	} else if (pid % 4 == 2) {/*
		idx1 = pop(stack);
		if (idx1 != 0) {
			a = atomic_load_explicit(&x[idx1], relaxed);
			printf("a: %d\n", a);
		}*/
	} else {
		idx2 = pop(&stack);
		if (idx2 != 0) {
			b = atomic_load_explicit(&x[idx2], relaxed);
			/* printf("b: %d\n", b); */
		}
	}
	return NULL;
}
