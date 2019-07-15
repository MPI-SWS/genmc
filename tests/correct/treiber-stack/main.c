#include "my_stack.c"

#define NUM_THREADS 4

static mystack_t stack;
static int num_threads = NUM_THREADS;
static pthread_t threads[NUM_THREADS];
static int param[NUM_THREADS];

unsigned int idx1, idx2;
unsigned int a, b;

atomic_int x[3];

int __thread tid;

void set_thread_num(int i)
{
	tid = i;
}

int get_thread_num()
{
	return tid;
}

void *main_task(void *param)
{
	unsigned int val;
	int pid = *((int *)param);

	set_thread_num(pid);

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
