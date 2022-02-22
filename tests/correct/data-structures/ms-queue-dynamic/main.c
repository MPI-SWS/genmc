#include "my_queue.c"

#ifndef MAX_THREADS
# define MAX_THREADS 32
#endif

#ifdef CONFIG_QUEUE_READERS
#define DEFAULT_READERS (CONFIG_QUEUE_READERS)
#else
#define DEFAULT_READERS 0
#endif

#ifdef CONFIG_QUEUE_WRITERS
#define DEFAULT_WRITERS (CONFIG_QUEUE_WRITERS)
#else
#define DEFAULT_WRITERS 2
#endif

#ifdef CONFIG_QUEUE_RDWR
#define DEFAULT_RDWR (CONFIG_QUEUE_RDWR)
#else
#define DEFAULT_RDWR 0
#endif

int readers = DEFAULT_READERS, writers = DEFAULT_WRITERS, rdwr = DEFAULT_RDWR;

static queue_t *queue;
static int num_threads;

queue_t myqueue;
int param[MAX_THREADS];
unsigned int input[MAX_THREADS];
unsigned int output[MAX_THREADS];
pthread_t threads[MAX_THREADS];

int __thread tid;

void set_thread_num(int i)
{
	tid = i;
}

int get_thread_num()
{
	return tid;
}

bool succ[MAX_THREADS];

void *threadW(void *param)
{
	unsigned int val;
	int pid = *((int *)param);

	set_thread_num(pid);

	input[pid] = pid * 10;
	enqueue(queue, input[pid]);
//	succ[pid] = dequeue(queue, &output[pid]);
	//printf("Dequeue: %d\n", output[0]);
	return NULL;
}

void *threadR(void *param)
{
	unsigned int val;
	int pid = *((int *)param);

	set_thread_num(pid);

//	input[pid] = pid * 10;
//	enqueue(queue, input[pid]);
	succ[pid] = dequeue(queue, &output[pid]);
	//printf("Dequeue: %d\n", output[0]);
	return NULL;
}

void *threadRW(void *param)
{
	unsigned int val;
	int pid = *((int *)param);

	set_thread_num(pid);

	input[pid] = pid * 10;
	enqueue(queue, input[pid]);
	succ[pid] = dequeue(queue, &output[pid]);
	//printf("Dequeue: %d\n", output[0]);
	return NULL;
}
