#include "combiner.c"
#include "set.c"

/* Client code */
#ifndef MAX_THREADS
# define MAX_THREADS 32
#endif
#ifndef MAX_FREELIST
# define MAX_FREELIST 32 /* Each thread can own up to MAX_FREELIST free nodes */
#endif

#ifndef N_THREADS
# define N_THREADS 2
#endif

static int num_threads = N_THREADS + 1;

pthread_t threads[MAX_THREADS + 1];
int param[MAX_THREADS + 1];
struct set_node free_lists[MAX_THREADS + 1][MAX_FREELIST];
unsigned int free_index[MAX_THREADS + 1];

int __thread tid;
atomic_int gtid;

void set_thread_num(int i)
{
	tid = i;
}

int get_thread_num()
{
	return tid;
}

struct set_node *new_node(int key, int elem)
{
	int t = get_thread_num();

	assert(free_index[t] < MAX_FREELIST);
	free_lists[t][free_index[t]].key = key;
	free_lists[t][free_index[t]].val = elem;
	return &free_lists[t][free_index[t]++];
}

typedef struct {
	struct combine_message msg;
	int to_add;
	int was_called;
} locked_val_msg;

static struct set_head myset;
int gval = 0;
int num_run = 1;

void init()
{
	param[0] = 0;
	for (int j = 1; j < num_threads; j++)
		param[j] = j;

	set_init(&myset);
	for (int i = 0; i < 2; i++)
		set_add(&myset, -i);
}

void add_set(struct combine_message *val)
{
	locked_val_msg *msg = (locked_val_msg *) val;
	set_add(&myset, msg->to_add);
	msg->was_called += 1;
}

void *thread_n(void *data)
{
	int mtid = atomic_fetch_add_explicit(&gtid, 1, memory_order_seq_cst);
	set_thread_num(mtid);
	/* printf("Entering %d\n", mtid); */

	struct combiner *cmb = (struct combiner *) data;

	for (int i = 0; i < num_run; i++) {
		locked_val_msg m;
		m.msg.prefetch = &myset;
		m.was_called = 0;
		m.to_add = tid;
		m.msg.operation = &add_set;
		message_combiner(cmb, &m.msg);
		assert(m.was_called == 1);
	}

	/* printf("Exiting %d\n", mtid); */
	return NULL;
}
