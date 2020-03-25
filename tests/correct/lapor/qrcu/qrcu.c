#include "qrcu.h"

int init_qrcu_struct(struct qrcu_struct *qp)
{
	qp->completed = 0;
	atomic_set(qp->ctr + 0, 1);
	atomic_set(qp->ctr + 1, 0);
	init_waitqueue_head(&qp->wq);
	mutex_init(&qp->mutex);

	return 0;
}

int qrcu_read_lock(struct qrcu_struct *qp)
{
	for (;;) {
		int idx = qp->completed & 0x1;
		if (likely(atomic_inc_not_zero(qp->ctr + idx)))
			return idx;
	}
}

void qrcu_read_unlock(struct qrcu_struct *qp, int idx)
{
	if (atomic_dec_and_test(qp->ctr + idx))
		wake_up(&qp->wq);
}

void synchronize_qrcu(struct qrcu_struct *qp)
{
	int idx;

	smp_mb();  /* Force preceding change to happen before fastpath check. */

	/*
	 * Fastpath: If the two counters sum to "1" at a given point in
	 * time, there are no readers.  However, it takes two separate
	 * loads to sample both counters, which won't occur simultaneously.
	 * So we might race with a counter switch, so that we might see
	 * ctr[0]==0, then the counter might switch, then we might see
	 * ctr[1]==1 (unbeknownst to us because there is a reader still
	 * there).  So we do a read memory barrier and recheck.  If the
	 * same race happens again, there must have been a second counter
	 * switch.  This second counter switch could not have happened
	 * until all preceding readers finished, so if the condition
	 * is true both times, we may safely proceed.
	 *
	 * This relies critically on the atomic increment and atomic
	 * decrement being seen as executing in order.
	 */

	if (atomic_read(&qp->ctr[0]) + atomic_read(&qp->ctr[1]) <= 1) {
		smp_rmb();  /* Keep two checks independent. */
		if (atomic_read(&qp->ctr[0]) + atomic_read(&qp->ctr[1]) <= 1)
			goto out;
	}

	mutex_lock(&qp->mutex);

	idx = qp->completed & 0x1;
	if (atomic_read(qp->ctr + idx) == 1)
		goto out_unlock;

	atomic_inc(qp->ctr + (idx ^ 0x1));

	/*
	 * Prevent subsequent decrement from being seen before previous
	 * increment -- such an inversion could cause the fastpath
	 * above to falsely conclude that there were no readers.  Also,
	 * reduce the likelihood that qrcu_read_lock() will loop.
	 */

	smp_mb__after_atomic_inc();
	qp->completed++;

	atomic_dec(qp->ctr + idx);
	__wait_event(qp->wq, !atomic_read(qp->ctr + idx));
out_unlock:
	mutex_unlock(&qp->mutex);
out:
	smp_mb(); /* force subsequent free after qrcu_read_unlock(). */
}

EXPORT_SYMBOL_GPL(init_qrcu_struct);
EXPORT_SYMBOL_GPL(qrcu_read_lock);
EXPORT_SYMBOL_GPL(qrcu_read_unlock);
EXPORT_SYMBOL_GPL(synchronize_qrcu);
