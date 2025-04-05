#include <pthread.h>
#include <stdlib.h>
#include <genmc.h>

#include "my_queue.h"

#ifdef MAKE_ACCESSES_SC
# define relaxed memory_order_seq_cst
# define release memory_order_seq_cst
# define acquire memory_order_seq_cst
#else
# define relaxed memory_order_relaxed
# define release memory_order_release
# define acquire memory_order_acquire
#endif

typedef struct node {
	unsigned int value;
	_Atomic(struct node *) next;
} node_t;

typedef struct queue {
	_Atomic(node_t *) head;
	_Atomic(node_t *) tail;
} queue_t;

static node_t *new_node()
{
	return malloc(sizeof(node_t));
}

static void reclaim(node_t *node)
{
	__VERIFIER_hp_retire(node);
}

void init_queue(queue_t *q, int num_threads)
{
	node_t *dummy = new_node();
	atomic_init(&dummy->next, NULL);
	atomic_init(&q->head, dummy);
	atomic_init(&q->tail, dummy);
}

void clear_queue(queue_t *q, int num_threads)
{
	node_t *next;
	while (q->head != NULL) {
		// int v = q->head->value;
		next = q->head->next;
		free(q->head);
		q->head = next;
	}
}


void enqueue(queue_t *q, unsigned int val)
{
	node_t *tail, *next;

	node_t *node = new_node();
	node->value = val;
	node->next = NULL;

	__VERIFIER_hp_t *hp = __VERIFIER_hp_alloc();
	while (true) {
		tail = __VERIFIER_hp_protect(hp, &q->tail);
		next = atomic_load_explicit(&tail->next, acquire);
		if (tail != atomic_load_explicit(&q->tail, acquire))
			continue;

		if (next == NULL) {
			if (__VERIFIER_final_CAS(
				    atomic_compare_exchange_strong_explicit(&tail->next, &next,
									    node, release, relaxed)))
				break;
		} else {
			__VERIFIER_helping_CAS(
				atomic_compare_exchange_strong_explicit(&q->tail, &tail, next,
									release, relaxed);
			);
		}
	}
	__VERIFIER_helped_CAS(
		atomic_compare_exchange_strong_explicit(&q->tail, &tail, node, release, relaxed);
	);
	__VERIFIER_hp_free(hp);
}

bool dequeue(queue_t *q, unsigned int *retVal)
{
	node_t *head, *tail, *next;
	unsigned ret;

	__VERIFIER_hp_t *hp_head = __VERIFIER_hp_alloc();
	__VERIFIER_hp_t *hp_next = __VERIFIER_hp_alloc();
	while (true) {
		head = __VERIFIER_hp_protect(hp_head, &q->head);
		tail = atomic_load_explicit(&q->tail, acquire);
		next = __VERIFIER_hp_protect(hp_next, &head->next);
		if (atomic_load_explicit(&q->head, acquire) != head)
			continue;
		if (head == tail) {
			if (next == NULL) {
				__VERIFIER_hp_free(hp_head);
				__VERIFIER_hp_free(hp_next);
				return false;
			}
			__VERIFIER_helping_CAS(
				atomic_compare_exchange_strong_explicit(&q->tail, &tail, next,
									release, relaxed);
			);
		} else {
			ret = next->value;
			if (atomic_compare_exchange_strong_explicit(&q->head, &head, next,
								    release, relaxed))
				break;
		}
	}
	*retVal = ret;
	reclaim(head);
	__VERIFIER_hp_free(hp_head);
	__VERIFIER_hp_free(hp_next);
	return true;
}
