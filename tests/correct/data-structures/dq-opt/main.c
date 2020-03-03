#include "dq.c"

/***********************************************************
 * Client infrastructure
 ***********************************************************/

#ifndef PUSH_T1_BEFORE
# define PUSH_T1_BEFORE 2
#endif
#ifndef POP_T1
# define POP_T1 1
#endif
#ifndef PUSH_T1_AFTER
# define PUSH_T1_AFTER 1
#endif
#ifndef STEAL_T1
# define STEAL_T1 0
#endif

#ifndef PUSH_T2_BEFORE
# define PUSH_T2_BEFORE 0
#endif
#ifndef POP_T2
# define POP_T2 0
#endif
#ifndef PUSH_T2_AFTER
# define PUSH_T2_AFTER 0
#endif
#ifndef STEAL_T2
# define STEAL_T2 2
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
#ifndef STEAL_T3
# define STEAL_T3 1
#endif

struct deque mydeque;

void *thread_1(void *arg)
{
	int64_t count = 1;
	int64_t data, res;
        int64_t result1, result2, result3, result4;

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T1_BEFORE; i++) {
		if (deque_try_push(&mydeque, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result1 = res;

	/* pop some items */
	res = 0;
	for (int i = 0; i < POP_T1; i++) {
		if (deque_try_pop(&mydeque, &data) >= 0) {
			res += data;
		}
	}
	result2 = res;

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T1_AFTER; i++) {
		if (deque_try_push(&mydeque, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result3 = res;

	/* steal some items */
	res = 0;
	for (int i = 0; i < STEAL_T1; i++) {
		if (deque_try_steal(&mydeque, &data) >= 0) {
			res += data;
		}
	}
	result4 = res;

	return NULL;
}

void *thread_2(void *arg)
{
	int64_t count = 1;
	int64_t data, res;
	int64_t result1, result2, result3, result4;

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T2_BEFORE; i++) {
		if (deque_try_push(&mydeque, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result1 = res;

	/* pop some items */
	res = 0;
	for (int i = 0; i < POP_T2; i++) {
		if (deque_try_pop(&mydeque, &data) >= 0) {
			res += data;
		}
	}
	result2 = res;

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T2_AFTER; i++) {
		if (deque_try_push(&mydeque, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result3 = res;

	/* steal some items */
	res = 0;
	for (int i = 0; i < STEAL_T2; i++) {
		if (deque_try_steal(&mydeque, &data) >= 0) {
			res += data;
		}
	}
	result4 = res;

	return NULL;
}

void *thread_3(void *arg)
{
	int64_t count = 1;
	int64_t data, res;
	int64_t result1, result2, result3, result4;

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T3_BEFORE; i++) {
		if (deque_try_push(&mydeque, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result1 = res;

	/* pop some items */
	res = 0;
	for (int i = 0; i < POP_T3; i++) {
		if (deque_try_pop(&mydeque, &data) >= 0) {
			res += data;
		}
	}
	result2 = res;

	/* push some items */
	res = 0;
	for (int i = 0; i < PUSH_T3_AFTER; i++) {
		if (deque_try_push(&mydeque, count) >= 0) {
			res += count;
			count *= 2;
		}
	}
	result3 = res;

	/* steal some items */
	res = 0;
	for (int i = 0; i < STEAL_T3; i++) {
		if (deque_try_steal(&mydeque, &data) >= 0) {
			res += data;
		}
	}
	result4 = res;

	return NULL;
}
