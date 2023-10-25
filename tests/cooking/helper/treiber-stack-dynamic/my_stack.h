#define MAX_NODES			0xff

typedef struct node {
	unsigned int value;
	_Atomic(struct node *) next;

} node_t;

typedef struct {
	_Atomic(node_t *) top;
	node_t nodes[MAX_NODES + 1];
} mystack_t;

void init_stack(mystack_t *s, int num_threads);
void push(mystack_t *s, unsigned int val);
unsigned int pop(mystack_t *s);
int get_thread_num();
