/***********************************************************
 * Queue implementation (utilizes malloc infrastructure)
 ***********************************************************/

#ifndef LEN
# define LEN 10
#endif

struct deque {
	atomic_uint_fast64_t bottom;
	atomic_uint_fast64_t top;
	_Atomic int64_t buffer[LEN]; // in fact, it should be marked as atomic
	//due to the race between push and
	// steal.
};

int64_t deque_try_push(struct deque *deq, int64_t data)
{
	uint64_t b = atomic_load_explicit(&deq->bottom, memory_order_relaxed);
	uint64_t t = atomic_load_explicit(&deq->top, memory_order_relaxed);

	int64_t len = (int64_t) (b - t);
	if (len >= LEN) {
		return -1; // full
	}

	atomic_store_explicit(&deq->buffer[b % LEN], data, memory_order_relaxed);
	atomic_store_explicit(&deq->bottom, b + 1, memory_order_release);
	return 0;
}

int64_t deque_try_pop(struct deque *deq, int64_t *data)
{
	uint64_t b = atomic_load_explicit(&deq->bottom, memory_order_relaxed);
	atomic_store_explicit(&deq->bottom, b - 1, memory_order_relaxed);

	atomic_thread_fence(memory_order_seq_cst);

	uint64_t t = atomic_load_explicit(&deq->top, memory_order_relaxed);
	int64_t len = (int64_t) (b - t);

	if (len <= 0) {
		atomic_store_explicit(&deq->bottom, b, memory_order_relaxed);
		return -1; // empty
	}

	*data = atomic_load_explicit(&deq->buffer[(b - 1) % LEN], memory_order_relaxed);

	if (len > 1) {
		return 0;
	}

	// len = 1.
	bool is_successful = atomic_compare_exchange_strong_explicit(&deq->top, &t, t + 1,
							    memory_order_release,
							    memory_order_relaxed);
	atomic_store_explicit(&deq->bottom, b, memory_order_relaxed);
	return (is_successful ? 0 : -2); // success or lost
}

int64_t deque_try_steal(struct deque *deq, int64_t* data)
{
	uint64_t t = atomic_load_explicit(&deq->top, memory_order_relaxed);

	atomic_thread_fence(memory_order_seq_cst);

	uint64_t b = atomic_load_explicit(&deq->bottom, memory_order_acquire);
	int64_t len = (int64_t) (b - t);

	if (len <= 0) {
		return -1; // empty
	}

	*data = atomic_load_explicit(&deq->buffer[t % LEN], memory_order_relaxed);

	bool is_successful = atomic_compare_exchange_strong_explicit(&deq->top, &t, t + 1,
							    memory_order_release,
							    memory_order_relaxed);
	return (is_successful ? 0 : -2); // success or lost
}
