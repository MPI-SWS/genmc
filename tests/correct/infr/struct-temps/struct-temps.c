/* Makes sure we get rid of some temporaries */

typedef struct mcs_node_s {
	_Atomic(struct mcs_node_s*) next;
	atomic_int spin;
} mcs_node_t;

mcs_node_t node;

void *thread_1(void *unused)
{
	mcs_node_t *r = &node;
	while (atomic_load_explicit(&node.next, memory_order_seq_cst) == NULL)
		;
	while (atomic_load_explicit(&node.next, memory_order_seq_cst) !=
	       atomic_load_explicit(&r->next, memory_order_seq_cst))
		;

	return NULL;
}

void *thread_2(void *unused)
{
	node.next = 0xdeadbeef;
	return NULL;
}
