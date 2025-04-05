#include "my_queue.h"

#include <stdatomic.h>
#include <assert.h>
#include <genmc.h>

#ifdef MAKE_ACCESSES_SC
# define relaxed memory_order_seq_cst
# define release memory_order_seq_cst
# define acquire memory_order_seq_cst
#define acq_rel memory_order_seq_cst
#else
# define relaxed memory_order_relaxed
# define release memory_order_release
# define acquire memory_order_acquire
#define acq_rel memory_order_acq_rel
#endif

#ifndef MAX_THREADS
# define MAX_THREADS 32
#endif

/* Implementation of the Herlihy-Wing queue:
   Maurice Herlihy and Jeannette M. Wing. 1990. “Linearizability: A Correctness Condition for Concurrent Objects.” doi: 10.1145/78969.78972.
*/

void init_queue(queue_t *q, int num_threads) {
}

void clear_queue(queue_t *q, int num_threads) {

}

// @mut::function@
void enqueue(queue_t *q, unsigned int val) {
	int i = atomic_fetch_add_explicit(&q->tail, 1, release);
	assert(i + 1 < MAX_NODES);
	atomic_store_explicit(&q->nodes[i + 1], val, release);
}

// NOTE: blocking
// @mut::function@
bool dequeue(queue_t *q, unsigned int *ret) {
	bool success = false;
	while (!success) {
		int tail = atomic_load_explicit(&q->tail, acquire);
		for (int i = 0; i <= tail; ++i) {
			if (atomic_load_explicit(&q->nodes[i], acquire) == 0) /* resolve ww-race */
				continue;
#ifdef BUG // Linearizability bug
			unsigned int v = atomic_exchange_explicit(&q->nodes[i], 0, acquire);
#else
			unsigned int v = atomic_exchange_explicit(&q->nodes[i], 0, acq_rel);
#endif
			if (v != 0) {
				*ret = v;
				success = true;
				break;
			}
		}
		__VERIFIER_assume(success);
	}
	return *ret;
}

/* NOTE: In this implementation it's impossible to synchronize 'dequeue' methods
 *       between each other.
 * */
