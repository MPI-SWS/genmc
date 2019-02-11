#include "ordering.h"

#ifndef LEN
# define LEN 10
#endif

struct Deque {
	atomic_uint_fast64_t bottom;
	atomic_uint_fast64_t top;
	int64_t buffer[LEN]; // in fact, it should be marked as atomic
	//due to the race between push and
	// steal.
};

int64_t try_push(struct Deque *deq, int64_t N, int64_t data)
{
	uint64_t b = atomic_load_explicit(&deq->bottom, mo_rlx);
	uint64_t t = atomic_load_explicit(&deq->top, mo_acq);

	int64_t len = (int64_t) (b - t);
	if (len >= N) {
		return -1; // full
	}

	deq->buffer[b % N] = data;
	atomic_store_explicit(&deq->bottom, b + 1, mo_rel);
	return 0;
}

int64_t try_pop(struct Deque *deq, int64_t N, int64_t *data)
{
	uint64_t b = atomic_load_explicit(&deq->bottom, mo_rlx);
	atomic_store_explicit(&deq->bottom, b - 1, mo_rlx);

	atomic_thread_fence(mo_seq_cst);

	uint64_t t = atomic_load_explicit(&deq->top, mo_rlx);
	int64_t len = (int64_t) (b - t);

	if (len <= 0) {
		atomic_store_explicit(&deq->bottom, b, mo_rlx);
		return -1; // empty
	}

	*data = deq->buffer[(b - 1) % N];

	if (len > 1) {
		return 0;
	}

	// len = 1.
	bool is_successful = atomic_compare_exchange_strong_explicit(&deq->top, &t, t + 1,
							    mo_acq_rel,
							    mo_acq_rel);
	atomic_store_explicit(&deq->bottom, b, mo_rlx);
	return (is_successful ? 0 : -2); // success or lost
}

int64_t try_steal(struct Deque *deq, int64_t N, int64_t* data)
{
	uint64_t t = atomic_load_explicit(&deq->top, mo_rlx);

	atomic_thread_fence(mo_seq_cst);

	uint64_t b = atomic_load_explicit(&deq->bottom, mo_acq);
	int64_t len = (int64_t) (b - t);

	if (len <= 0) {
		return -1; // empty
	}

	*data = deq->buffer[t % N];

	bool is_successful = atomic_compare_exchange_strong_explicit(&deq->top, &t, t + 1,
							    mo_rel,
							    mo_rel);
	return (is_successful ? 0 : -2); // success or lost
}
