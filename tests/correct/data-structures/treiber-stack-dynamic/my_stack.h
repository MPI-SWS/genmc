#define MAX_NODES			0xff

typedef unsigned long long pointer;
typedef atomic_ullong pointer_t;

typedef struct node {
	unsigned int value;
	pointer_t next;

} node_t;

typedef struct {
	pointer_t top;
	node_t nodes[MAX_NODES + 1];
} mystack_t;

void init_stack(mystack_t *s, int num_threads);
void push(mystack_t *s, unsigned int val);
unsigned int pop(mystack_t *s);
int get_thread_num();
