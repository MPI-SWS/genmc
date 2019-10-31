#include <stdatomic.h>

typedef struct spinning_barrier {
	const unsigned int n_; /* Number of synchronized threads. */
	atomic_uint nwait_;    /* Number of threads currently spinning. */
	atomic_uint step_;     /* Number of barrier syncronizations completed
				* so far, it's OK to wrap. */
} barrier_t;

#define DEFINE_BARRIER(b, n) \
	barrier_t b = { .n_ = n, .nwait_ = 0, .step_ = 0 }

bool wait(barrier_t *b)
{
	unsigned int step = atomic_load(&b->step_);

	if (atomic_fetch_add(&b->nwait_, 1) == b->n_ - 1) {
		/* OK, last thread to come.  */
		atomic_store(&b->nwait_, 0); // XXX: maybe can use relaxed ordering here ??
		atomic_fetch_add(&b->step_, 1);
		return true;
	} else {
		while (atomic_load(&b->step_) == step)
			;
		return false;
	}
}
