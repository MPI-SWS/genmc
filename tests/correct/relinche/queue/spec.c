#include <stdatomic.h>
#include <stdlib.h>
#include <pthread.h>

#include <genmc.h>

#include "../helper.h"

// #include "queue.h"

/* Queue with specification only on matched value */

typedef struct _node_t {
	int value;
	_Atomic(struct _node_t *) next;
} node_t;

typedef struct _queue_t {
	_Atomic(node_t *) head;
	_Atomic(node_t *) tail;
#ifdef SYNC_INS
	_Atomic(int) enq_cnt;
#endif
#ifdef SYNC_REM
	_Atomic(int) deq_cnt;
#endif
} queue_t;

node_t *new_node() { return malloc(sizeof(node_t)); }

void reclaim(node_t *node) { free(node); }

__VERIFIER_plock_t gl;

void init_queue(queue_t *q, int num_thread) {
	__VERIFIER_plock_lock(&gl);
	node_t *dummy = new_node();
	dummy->value = -42;
	atomic_init(&dummy->next, NULL);
	atomic_init(&q->head, dummy);
	atomic_init(&q->tail, dummy);
	__VERIFIER_plock_unlock(&gl);
}

void clear_queue(queue_t *q, int num_thread) {
	__VERIFIER_plock_lock(&gl);
	node_t *it = q->head;
	node_t *it_next;
	while (it != NULL) {
		it_next = it->next;
		free(it);
		it = it_next;
	}
	__VERIFIER_plock_unlock(&gl);
}

void enqueue(queue_t *q, unsigned int val) {
	__VERIFIER_plock_lock(&gl);
#ifdef SYNC_INS
	FAA(&q->enq_cnt, 1, acq_rel);
#endif
	node_t *node = new_node();
	node->value = val;
	atomic_init(&node->next, NULL);
	node_t *tail = load(&q->tail, relaxed);
	store(&tail->next, node, release);
	store(&q->tail, node, relaxed);
	__VERIFIER_plock_unlock(&gl);
}

bool dequeue(queue_t *q, unsigned int *ret) {
	__VERIFIER_plock_lock(&gl);
	node_t *head = load(&q->head, relaxed);
	node_t *tail = load(&q->tail, relaxed);
	if (head == tail) {
		__VERIFIER_plock_unlock(&gl);
		return false;
	}
#ifdef SYNC_REM
	FAA(&q->deq_cnt, 1, acq_rel);
#endif
	head = load(&q->head, relaxed);
	node_t *next = load(&head->next, acquire);
	*ret = next->value;
	store(&q->head, next, relaxed);
	reclaim(head);
	__VERIFIER_plock_unlock(&gl);
	return true;
}
