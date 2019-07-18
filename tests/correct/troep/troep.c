#define THREADS 2

struct Node {
	struct Node* next;
};

_Atomic(struct Node*) list;

pthread_t t[THREADS];
struct Node n[THREADS];

void* producer_thread(void* param)
{
	struct Node* node;

	node =  (struct Node*)param;

	// Insert node at beginning of the list.
	node->next = atomic_load(&list);
	while (!atomic_compare_exchange_weak(&list, &node->next, node))
		;

	return NULL;
}
