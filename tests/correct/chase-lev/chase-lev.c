#include "deque.h"

#ifndef NUM
# define NUM 10
#endif

#ifdef CONFIG_DEQ_STEALERS
#define DEFAULT_STEALERS (CONFIG_DEQ_STEALERS)
#else
#define DEFAULT_STEALERS 1
#endif

void thread0(struct Deque *que, int64_t N, int64_t X1, int64_t X2, int64_t X3,
	     int64_t *result1, int64_t *result2, int64_t *result3)
{
	int64_t count = 1;
	int64_t data;
	int64_t res;

	res = 0;
	for (int64_t i = 0; i < X1; ++i) {
		if (try_push(que, N, count) >= 0) {
			res += count;
			count++;
		}
	}
	*result1 = res;

	res = 0;
	for (int64_t i = 0; i < X2; ++i) {
		if (try_pop(que, N, &data) >= 0) {
			res += data;
		}
	}
	*result2 = res;

	res = 0;
	for (int64_t i = 0; i < X3; ++i) {
		if (try_push(que, N, count) >= 0) {
			res += count;
			count++;
		}
	}
	*result3 = res;
}

void thread1(struct Deque *que, int64_t N, int64_t X, int64_t *result)
{
	int64_t data;
	int64_t res = 0;
	for (int64_t i = 0; i < X; ++i) {
		if (try_steal(que, N, &data) >= 0) {
			res += data;
		}
	}
	*result = res;
}

void thread2(struct Deque *que, int64_t N, int64_t X, int64_t *result)
{
	int64_t data;
	int64_t res = 0;
	for (int64_t i = 0; i < X; ++i) {
		if (try_steal(que, N, &data) >= 0) {
			res += data;
		}
	}
	*result = res;
}

struct Deque deq;

void *thread_pp(void *unused)
{
	int64_t res1, res2, res3;

	thread0(&deq, NUM, 5, 3, 2, &res1, &res2, &res3);
	return NULL;
}

void *thread_s(void *unused)
{
	int64_t res1;

	thread1(&deq, NUM, 1, &res1);
	return NULL;
}
