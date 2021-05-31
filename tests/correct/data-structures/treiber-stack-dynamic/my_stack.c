#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#include "my_stack.h"

#ifdef MAKE_ALL_SC
# define release memory_order_seq_cst
# define acquire memory_order_seq_cst
# define relaxed memory_order_seq_cst
#else
# define release memory_order_release
# define acquire memory_order_acquire
# define relaxed memory_order_relaxed
#endif

#ifndef MAX_THREADS
# define MAX_THREADS 32
#endif

#define POISON_IDX 0x666

static node_t *new_node()
{
	return malloc(sizeof(node_t));
}

static void reclaim(node_t *node)
{
	free(node);
}

void init_stack(mystack_t *s, int num_threads)
{
	/* initialize stack */
	atomic_init(&s->top, 0);
}

void push(mystack_t *s, unsigned int val)
{
	node_t *node = new_node();
	node->value = val;
	pointer oldTop, newTop;
	bool success;
	while (true) {
		// acquire
		oldTop = atomic_load_explicit(&s->top, acquire);
		newTop = (pointer) node;
		// relaxed
		atomic_store_explicit(&node->next, oldTop, relaxed);

		// release & relaxed
		success = atomic_compare_exchange_strong_explicit(&s->top, &oldTop,
			newTop, release, relaxed);
		if (success)
			break;
	}
}

unsigned int pop(mystack_t *s)
{
	pointer oldTop, newTop, next;
	node_t *node;
	bool success;
	int val;
	while (true) {
		// acquire
		oldTop = atomic_load_explicit(&s->top, acquire);
		if (oldTop == NULL)
			return 0;
		node = oldTop;
		// relaxed
		next = atomic_load_explicit(&node->next, relaxed);
		newTop = (pointer) next;
		// release & relaxed
		success = atomic_compare_exchange_strong_explicit(&s->top, &oldTop,
			newTop, release, relaxed);
		if (success)
			break;
	}
	val = node->value;
	/* Reclaim the used slot */
	reclaim(oldTop);
	return val;
}
