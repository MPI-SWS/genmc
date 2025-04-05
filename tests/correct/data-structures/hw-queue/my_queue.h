#define MAX_NODES			0xff
#include <stdbool.h>

typedef struct _queue_t {
	_Atomic(int) tail;
	_Atomic(unsigned int) nodes[MAX_NODES] ;
} queue_t;

void init_queue(queue_t *q, int num_threads);
void enqueue(queue_t *q, unsigned int val);
bool dequeue(queue_t *q, unsigned int *retVal);
int get_thread_num();
