#ifndef _LINUX_SRCU_H
#define _LINUX_SRCU_H

#include <linux/wait.h>

/*
 * fully compatible with srcu, but optimized for writers.
 */

struct qrcu_struct {
	atomic_int completed; /* originally plain int in the kernel */
	atomic_t ctr[2];
	wait_queue_head_t wq;
	struct mutex mutex;
};

int init_qrcu_struct(struct qrcu_struct *qp);
int qrcu_read_lock(struct qrcu_struct *qp);
void qrcu_read_unlock(struct qrcu_struct *qp, int idx);
void synchronize_qrcu(struct qrcu_struct *qp);

static inline void cleanup_qrcu_struct(struct qrcu_struct *qp)
{
}

#endif
