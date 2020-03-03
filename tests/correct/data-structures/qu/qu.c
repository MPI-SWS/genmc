/*
 * Michael-Scott queue -- adapted from [Pulte et al. 2019]
 */

/***********************************************************
 * Memory-allocation infrastructure -- implemented by client
 ***********************************************************/

int get_thread_num();
struct queue_node *new_node();


/***********************************************************
 * Queue implementation (utilizes malloc infrastructure)
 ***********************************************************/

struct queue_node {
	int data;
	_Atomic(struct queue_node *) next;
};

struct queue {
	struct queue_node init;
	_Atomic(struct queue_node *) head;
	_Atomic(struct queue_node *) tail;
	atomic_bool is_initialized;
};

void queue_init(struct queue *q)
{
	atomic_store_explicit(&q->init.next, NULL, memory_order_release);
	atomic_store_explicit(&q->head, &q->init, memory_order_release);
	atomic_store_explicit(&q->tail, &q->init, memory_order_release);
	atomic_store_explicit(&q->is_initialized, true, memory_order_release);
}

bool queue_is_initialized(struct queue *q)
{
	return atomic_load_explicit(&q->is_initialized, memory_order_acquire);
}

struct queue_node *queue_find_tail(struct queue *q)
{
	struct queue_node *node = atomic_load_explicit(&q->tail, memory_order_acquire);
	struct queue_node *next = atomic_load_explicit(&node->next, memory_order_relaxed);

	if (next == NULL)
		return node;

	atomic_store_explicit(&q->tail, next, memory_order_release);
	return NULL;
}

int queue_try_enq(struct queue *q, int data)
{
	struct queue_node *node = new_node();

	if (node == NULL) {
		return -3; /* OOM */
	}

	node->data = data;
	atomic_store_explicit(&node->next, NULL, memory_order_relaxed);

	struct queue_node *tail = NULL;
	do {
		tail = queue_find_tail(q);
	} while (tail == NULL);

	struct queue_node *v = NULL;
	if (atomic_compare_exchange_strong_explicit(&tail->next, &v, node,
						    memory_order_release,
						    memory_order_release)) {
		atomic_store_explicit(&q->tail, node, memory_order_release);
		return 0;
	}

	return -2; /* CAVEAT: memory leak */
}

int queue_try_deq(struct queue *q, int *ret_data)
{
	struct queue_node *head = atomic_load_explicit(&q->head, memory_order_acquire);
	struct queue_node *node = atomic_load_explicit(&head->next, memory_order_acquire);

	if (node == NULL)
		return -1;

	if (atomic_compare_exchange_strong_explicit(&q->head, &head, node,
						    memory_order_relaxed,
						    memory_order_relaxed)) {
		*ret_data = node->data;
		return 0; /* CAVEAT: memory leak */
	}

	return -2;
}
