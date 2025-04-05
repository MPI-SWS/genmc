#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

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
	__VERIFIER_hp_retire(node);
}

void init_stack(mystack_t *s, int num_threads)
{
	/* initialize stack */
	atomic_init(&s->top, 0);
}

void clear_stack(mystack_t *s, int num_threads)
{
	node_t *next;
	while (s->top != 0) {
		next = s->top->next;
		free(s->top);
		s->top = next;
	}
}

void push(mystack_t *s, unsigned int val)
{
	node_t *node = new_node();
	node->value = val;

	while (true) {
		node_t *top = atomic_load_explicit(&s->top, acquire);
		atomic_store_explicit(&node->next, top, relaxed);
		if (atomic_compare_exchange_strong_explicit(&s->top, &top, node, release, relaxed))
			break;
	}
}

unsigned int pop(mystack_t *s)
{
	node_t *top;
	int val;

	__VERIFIER_hp_t *hp = __VERIFIER_hp_alloc();
	while (true) {
		top = __VERIFIER_hp_protect(hp, &s->top);
		if (top == NULL) {
			__VERIFIER_hp_free(hp);
			return 0;
		}

		node_t *next = atomic_load_explicit(&top->next, relaxed);
		if(atomic_compare_exchange_strong_explicit(&s->top, &top, next, release, relaxed))
			break;
	}
	val = top->value;
	/* Reclaim the used slot */
	reclaim(top);
	__VERIFIER_hp_free(hp);
	return val;
}
