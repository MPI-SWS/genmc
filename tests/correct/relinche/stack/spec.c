#include <stdatomic.h>
#include <stdlib.h>
#include <pthread.h>

#include <genmc.h>

#include "../helper.h"

#define MAX_NODES 0xff

typedef struct _node_t {
	_Atomic(int) value;
	int next;
} node_t;

typedef struct _stack_t {
	int top;
	node_t nodes[MAX_NODES + 1];
	_Atomic(int) push_cnt;
	_Atomic(int) pop_cnt;
} mystack_t;

__VERIFIER_plock_t gl;

void init_stack(mystack_t *s, int num_threads) {
	__VERIFIER_plock_lock(&gl);
	s->top = 0;
	__VERIFIER_plock_unlock(&gl);
}

void clear_stack(mystack_t *s, int num_thread) {
	__VERIFIER_plock_lock(&gl);
	__VERIFIER_plock_unlock(&gl);
}

void push(mystack_t *s, unsigned int val) {
	__VERIFIER_plock_lock(&gl);
#ifdef SYNC_INS
	FAA(&s->push_cnt, 1, acq_rel);
#endif
	node_t *new_node = &s->nodes[s->top];
	store(&new_node->value, val, release);
	new_node->next = s->top;
	s->top += 1;
	__VERIFIER_plock_unlock(&gl);
}

unsigned int pop(mystack_t *s) {
	__VERIFIER_plock_lock(&gl);
	if (s->top == 0) {
		__VERIFIER_plock_unlock(&gl);
		return 0;
	}
#ifdef SYNC_REM
	FAA(&s->pop_cnt, 1, acq_rel);
#endif
	unsigned int ret = load(&s->nodes[s->top-1].value, acquire);
	s->top -= 1;
	__VERIFIER_plock_unlock(&gl);
	return ret;
}
