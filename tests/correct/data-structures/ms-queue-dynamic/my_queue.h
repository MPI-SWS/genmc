#include <stdatomic.h>

struct queue;
typedef struct queue queue_t;

void init_queue(queue_t *q, int num_threads);
void enqueue(queue_t *q, unsigned int val);
bool dequeue(queue_t *q, unsigned int *retVal);
