#include "my_stack.c"

#define MAXREADERS 3
#define MAXWRITERS 3
#define MAXRDWR 3

#ifdef CONFIG_STACK_READERS
#define DEFAULT_READERS (CONFIG_STACK_READERS)
#else
#define DEFAULT_READERS 1
#endif

#ifdef CONFIG_STACK_WRITERS
#define DEFAULT_WRITERS (CONFIG_STACK_WRITERS)
#else
#define DEFAULT_WRITERS 2
#endif

#ifdef CONFIG_STACK_RDWR
#define DEFAULT_RDWR (CONFIG_STACK_RDWR)
#else
#define DEFAULT_RDWR 0
#endif

int readers = DEFAULT_READERS, writers = DEFAULT_WRITERS, rdwr = DEFAULT_RDWR;

static int num_threads;

static mystack_t stack;
static pthread_t threads[MAX_THREADS];
static int param[MAX_THREADS];

unsigned int idx1, idx2;
unsigned int a, b;

atomic_int x[MAX_THREADS];

int __thread tid;

void set_thread_num(int i)
{
	tid = i;
}

int get_thread_num()
{
	return tid;
}

void *threadW(void *param)
{
	unsigned int val;
	int pid = *((int *)param);

	set_thread_num(pid);

	atomic_store_explicit(&x[pid], pid + 42, relaxed);
	push(&stack, pid);
	return NULL;
}

void *threadR(void *param)
{
	unsigned int val;
	int pid = *((int *)param);

	set_thread_num(pid);

	int idx = pop(&stack);
	if (idx != 0) {
		int b = atomic_load_explicit(&x[idx], relaxed);
		/* printf("b: %d\n", b); */
	}
	return NULL;
}

void *threadRW(void *param)
{
	unsigned int val;
	int pid = *((int *)param);

	set_thread_num(pid);

	atomic_store_explicit(&x[pid], pid + 42, relaxed);
	push(&stack, pid);

	int idx = pop(&stack);
	if (idx != 0) {
		int b = atomic_load_explicit(&x[idx], relaxed);
		/* printf("b: %d\n", b); */
	}
	return NULL;
}
