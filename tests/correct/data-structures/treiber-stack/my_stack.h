#define MAX_NODES			0xff

typedef unsigned long long pointer;
typedef atomic_ullong pointer_t;

#define MAKE_POINTER(ptr, count)	((((pointer)count) << 32) | ptr)
#define PTR_MASK 0xffffffffLL
#define COUNT_MASK (0xffffffffLL << 32)

static inline void set_count(pointer *p, unsigned int val) { *p = (*p & ~COUNT_MASK) | ((pointer)val << 32); }
static inline void set_ptr(pointer *p, unsigned int val) { *p = (*p & ~PTR_MASK) | val; }
static inline unsigned int get_count(pointer p) { return (p & COUNT_MASK) >> 32; }
static inline unsigned int get_ptr(pointer p) { return p & PTR_MASK; }

typedef struct node {
	unsigned int value;
	pointer_t next;

} node_t;

typedef struct {
	pointer_t top;
	node_t nodes[MAX_NODES + 1];
} mystack_t;

void init_stack(mystack_t *s, int num_threads);
void clear_stack(mystack_t *s, int num_threads);
void push(mystack_t *s, unsigned int val);
unsigned int pop(mystack_t *s);
int get_thread_num();
