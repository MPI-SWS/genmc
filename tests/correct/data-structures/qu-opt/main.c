#include "qu.c"

/***********************************************************
 * Client infrastructure
 ***********************************************************/

#ifndef MAX_THREADS
# define MAX_THREADS 32
#endif
#ifndef MAX_FREELIST
# define MAX_FREELIST 32 /* Each thread can own up to MAX_FREELIST free nodes */
#endif

int __thread tid; /* Needs to be set before each thread starts executing */

int get_thread_num()
{
	return tid;
}

unsigned int idx[MAX_THREADS + 1];
struct queue_node free_lists[MAX_THREADS + 1][MAX_FREELIST];
unsigned int free_index[MAX_THREADS + 1];

struct queue_node *new_node()
{
	int t = get_thread_num();

	assert(free_index[t] < MAX_FREELIST);
	return &free_lists[t][free_index[t]++];
}

#ifndef ENQ_T1_BEFORE
# define ENQ_T1_BEFORE 1
#endif
#ifndef DEQ_T1
# define DEQ_T1 0
#endif
#ifndef ENQ_T1_AFTER
# define ENQ_T1_AFTER 0
#endif

#ifndef ENQ_T2_BEFORE
# define ENQ_T2_BEFORE 1
#endif
#ifndef DEQ_T2
# define DEQ_T2 0
#endif
#ifndef ENQ_T2_AFTER
# define ENQ_T2_AFTER 0
#endif

#ifndef ENQ_T3_BEFORE
# define ENQ_T3_BEFORE 0
#endif
#ifndef DEQ_T3
# define DEQ_T3 1
#endif
#ifndef ENQ_T3_AFTER
# define ENQ_T3_AFTER 0
#endif

struct queue myqueue;

void *thread_1(void *arg)
{
	int count = 1;
	int data, res;
	int result1, result2, result3;

	/* set TID */
	tid = *((int *) arg);

	/* Initialize queue within thread
	 * following Pulte et al's test case */
	queue_init(&myqueue);

	/* push some items */
	res = 0;
	for (int i = 0; i < ENQ_T1_BEFORE; i++) {
		if (queue_try_enq(&myqueue, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result1 = res;

	/* pop some items */
	res = 0;
	for (int i = 0; i < DEQ_T1; i++) {
		if (queue_try_deq(&myqueue, &data) >= 0) {
			res += data;
		}
	}
	result2 = res;

	/* push some items */
	res = 0;
	for (int i = 0; i < ENQ_T1_AFTER; i++) {
		if (queue_try_enq(&myqueue, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result3 = res;

	return NULL;
}

void *thread_2(void *arg)
{
	int count = 1;
	int data, res;
	int result1, result2, result3;

	/* set TID */
	tid = *((int *) arg);

	if (!queue_is_initialized(&myqueue))
		return NULL;

	/* push some items */
	res = 0;
	for (int i = 0; i < ENQ_T2_BEFORE; i++) {
		if (queue_try_enq(&myqueue, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result1 = res;

	/* pop some items */
	res = 0;
	for (int i = 0; i < DEQ_T2; i++) {
		if (queue_try_deq(&myqueue, &data) >= 0) {
			res += data;
		}
	}
	result2 = res;

	/* push some items */
	res = 0;
	for (int i = 0; i < ENQ_T2_AFTER; i++) {
		if (queue_try_enq(&myqueue, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result3 = res;

	return NULL;
}

void *thread_3(void *arg)
{
	int count = 1;
	int data, res;
	int result1, result2, result3;

	/* set TID */
	tid = *((int *) arg);

	if (!queue_is_initialized(&myqueue))
		return NULL;

	/* push some items */
	res = 0;
	for (int i = 0; i < ENQ_T3_BEFORE; i++) {
		if (queue_try_enq(&myqueue, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result1 = res;

	/* pop some items */
	res = 0;
	for (int i = 0; i < DEQ_T3; i++) {
		if (queue_try_deq(&myqueue, &data) >= 0) {
			res += data;
		}
	}
	result2 = res;

	/* push some items */
	res = 0;
	for (int i = 0; i < ENQ_T3_AFTER; i++) {
		if (queue_try_enq(&myqueue, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result3 = res;

	return NULL;
}
