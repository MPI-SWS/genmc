#ifndef __FINE_HEAP_H__
#define __FINE_HEAP_H__

#include <stdbool.h>

/* API that the Driver must implement */
int get_thread_num();

/* Heap implementation */
#define LOCK(l)   pthread_mutex_lock(&(l))
#define UNLOCK(l) pthread_mutex_unlock(&(l))

enum node_status { EMPTY, AVAILABLE, BUSY };

struct heap_node {
	enum node_status tag;
	int item;
	int score;
	int owner;
	pthread_mutex_t lock;
};

#define MAX_CAPACITY 512

#define ROOT   (1)
#define NO_ONE (-1)

struct heap {
	int next;
	struct heap_node nodes[MAX_CAPACITY + ROOT];
	pthread_mutex_t lock;
};

#define __HEAP_INITIALIZER()				\
	{ .next = ROOT					\
	, .lock = PTHREAD_MUTEX_INITIALIZER }

#define DEFINE_HEAP(heapname)			\
	struct heap heapname = __HEAP_INITIALIZER();

#define SWAP(x, y)				\
	do {					\
		typeof(x) SWAP = x;		\
		x = y;				\
		y = SWAP;			\
	} while (0)

bool is_curr_task_owner(struct heap_node *node)
{
	return node->tag == BUSY && node->owner == get_thread_num();
}

void add(struct heap *heap, int item, int score)
{
	LOCK(heap->lock);
	int child = heap->next++;
	LOCK(heap->nodes[child].lock);
	heap->nodes[child].tag = BUSY;
	heap->nodes[child].item = item;
	heap->nodes[child].score = score;
	heap->nodes[child].owner = get_thread_num();
	UNLOCK(heap->lock);
	UNLOCK(heap->nodes[child].lock);

	while (child > ROOT) {
		int parent = child / 2;
		LOCK(heap->nodes[parent].lock);
		LOCK(heap->nodes[child].lock);
		int old_child = child;

		if (heap->nodes[parent].tag == AVAILABLE &&
		    is_curr_task_owner(&heap->nodes[child])) {
			if (heap->nodes[child].score < heap->nodes[parent].score) {
				SWAP(heap->nodes[child], heap->nodes[parent]);
				child = parent;
			} else {
				heap->nodes[child].tag = AVAILABLE;
				heap->nodes[child].owner = NO_ONE;
				UNLOCK(heap->nodes[old_child].lock);
				UNLOCK(heap->nodes[parent].lock);
				return;
			}
		} else if (!is_curr_task_owner(&heap->nodes[child])) {
			child = parent;
		}

		UNLOCK(heap->nodes[old_child].lock);
		UNLOCK(heap->nodes[parent].lock);
	}

	if (child == ROOT) {
		LOCK(heap->nodes[ROOT].lock);
		if (is_curr_task_owner(&heap->nodes[ROOT])) {
			heap->nodes[ROOT].tag = AVAILABLE;
			heap->nodes[child].owner = NO_ONE;
		}
		UNLOCK(heap->nodes[ROOT].lock);
	}
}

#endif /* __FINE_HEAP_H__ */
