/*
 * Treiber stack -- adapted from [Pulte et al. 2019]
 */

/***********************************************************
 * Memory-allocation infrastructure -- implemented by client
 ***********************************************************/

int get_thread_num();
struct stack_node *new_node();


/***********************************************************
 * Stack implementation (utilizes malloc infrastructure)
 ***********************************************************/

struct stack_node {
	atomic_int data;
	_Atomic(struct stack_node *) next;
};

struct stack {
	_Atomic(struct stack_node *) head;
};

#define __STACK_INITIALIZER() { .head = ATOMIC_VAR_INIT(NULL) }
#define DEFINE_STACK(name)			\
	struct stack name = __STACK_INITIALIZER();

void stack_init(struct stack *s)
{
	atomic_store_explicit(&s->head, NULL, memory_order_seq_cst);
}

/* Returns -2 if lost, -3 if OOM */
int stack_try_push(struct stack *s, int data)
{
	struct stack_node *node = new_node();

	if (node == NULL)
		return -3; /* OOM */

	atomic_store_explicit(&node->data, data, memory_order_relaxed);
	struct stack_node *head = atomic_load_explicit(&s->head, memory_order_relaxed);
	atomic_store_explicit(&node->next, head, memory_order_relaxed);

	bool success = atomic_compare_exchange_strong_explicit(&s->head, &head, node,
							       memory_order_release,
							       memory_order_release);
	return (success ? 0 : -2); /* CAVEAT: memory leak if unsuccessful*/
}

/* Returns -1 if empty, -2 if lost */
int stack_try_pop(struct stack *s, int *ret_data)
{
	struct stack_node *head = atomic_load_explicit(&s->head, memory_order_acquire);

	if (head == NULL)
		return -1; /* empty */

	*ret_data = atomic_load_explicit(&head->data, memory_order_relaxed);
	struct stack_node *next = atomic_load_explicit(&head->next, memory_order_relaxed);
	bool success = atomic_compare_exchange_strong_explicit(&s->head, &head, next,
							       memory_order_release,
							       memory_order_release);
	return (success ? 0 : -2); /* CAVEAT: memory leak if successful */
}
