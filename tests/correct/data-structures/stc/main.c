#include "stc.c"

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
struct stack_node free_lists[MAX_THREADS + 1][MAX_FREELIST];
unsigned int free_index[MAX_THREADS + 1];

struct stack_node *new_node()
{
	int t = get_thread_num();

	assert(free_index[t] < MAX_FREELIST);
	return &free_lists[t][free_index[t]++];
}

#ifndef PUSH_T1_BEFORE
# define PUSH_T1_BEFORE 2
#endif
#ifndef POP_T1
# define POP_T1 1
#endif
#ifndef PUSH_T1_AFTER
# define PUSH_T1_AFTER 0
#endif

#ifndef PUSH_T2_BEFORE
# define PUSH_T2_BEFORE 0
#endif
#ifndef POP_T2
# define POP_T2 1
#endif
#ifndef PUSH_T2_AFTER
# define PUSH_T2_AFTER 1
#endif

#ifndef PUSH_T3_BEFORE
# define PUSH_T3_BEFORE 0
#endif
#ifndef POP_T3
# define POP_T3 0
#endif
#ifndef PUSH_T3_AFTER
# define PUSH_T3_AFTER 0
#endif

DEFINE_STACK(mystack);

void *thread_1(void *arg)
{
	int count = 1;
	int data, res;
	int result1, result2, result3;

	/* set TID */
	tid = *((int *) arg);

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T1_BEFORE; i++) {
		if (stack_try_push(&mystack, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result1 = res;

	/* pop some items */
	res = 0;
	for (int i = 0; i < POP_T1; i++) {
		if (stack_try_pop(&mystack, &data) >= 0) {
			res += data;
		}
	}
	result2 = res;

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T1_AFTER; i++) {
		if (stack_try_push(&mystack, count) >= 0) {
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

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T2_BEFORE; i++) {
		if (stack_try_push(&mystack, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result1 = res;

	/* pop some items */
	res = 0;
	for (int i = 0; i < POP_T2; i++) {
		if (stack_try_pop(&mystack, &data) >= 0) {
			res += data;
		}
	}
	result2 = res;

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T2_AFTER; i++) {
		if (stack_try_push(&mystack, count) >= 0) {
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

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T3_BEFORE; i++) {
		if (stack_try_push(&mystack, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result1 = res;

	/* pop some items */
	res = 0;
	for (int i = 0; i < POP_T3; i++) {
		if (stack_try_pop(&mystack, &data) >= 0) {
			res += data;
		}
	}
	result2 = res;

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T3_AFTER; i++) {
		if (stack_try_push(&mystack, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result3 = res;

	return NULL;
}
