#include <assert.h>
#include "fine-bst.c"

/* Driver code */
#ifndef MAX_THREADS
# define MAX_THREADS 32
#endif
#ifndef MAX_FREELIST
# define MAX_FREELIST 32 /* Each thread can own up to MAX_FREELIST free nodes */
#endif

#ifdef CONFIG_BST_ADDERS
# define DEFAULT_ADDERS (CONFIG_BST_ADDERS)
#else
# define DEFAULT_ADDERS 0
#endif
#ifdef CONFIG_BST_SEEKERS
# define DEFAULT_SEEKERS (CONFIG_BST_SEEKERS)
#else
# define DEFAULT_SEEKERS 2
#endif
#ifdef CONFIG_BST_REMOVERS
# define DEFAULT_REMOVERS (CONFIG_BST_REMOVERS)
#else
# define DEFAULT_REMOVERS 0
#endif

static int adders = DEFAULT_ADDERS, seekers = DEFAULT_SEEKERS, removers = DEFAULT_REMOVERS;
static int num_threads;

DEFINE_BST(mybst);

pthread_t threads[MAX_THREADS + 1];
int param[MAX_THREADS + 1];
struct bst_node free_lists[MAX_THREADS + 1][MAX_FREELIST];
unsigned int free_index[MAX_THREADS + 1];

int __thread tid;

void set_thread_num(int i)
{
	tid = i;
}

int get_thread_num()
{
	return tid;
}

struct bst_node *new_node(int elem)
{
	int t = get_thread_num();

	assert(free_index[t] < MAX_FREELIST);
	free_lists[t][free_index[t]].val = elem;
	free_lists[t][free_index[t]].left = NULL;
	free_lists[t][free_index[t]].right = NULL;
	return &free_lists[t][free_index[t]++];
}

void free_node(struct bst_node *node)
{
}

/* Should be called before threads are spawned (from main()) */
void init()
{
	num_threads = adders + seekers + removers + 1;
	for (int j = 0; j < num_threads; j++)
		param[j] = j;

	/* add(&mybst, 15); */
	/* add(&mybst, 11); */
	/* add(&mybst, 20); */
	/* add(&mybst, 18); */
	add(&mybst, 8);
	add(&mybst, 4);
	add(&mybst, 12);
	add(&mybst, 10);
}

#define BASE(tid) (((tid) % 2 == 0) ? tid : tid + 8)

void *thread_add(void *tid)
{
	int t = (*(int *) tid);
	set_thread_num(t);

	add(&mybst, (BASE(t) * 7) % 16);
	return NULL;
}

void *thread_seek(void *tid)
{
	int t = (*(int *) tid);
	set_thread_num(t);

	contains(&mybst, (BASE(t) * 7) % 16);
	return NULL;
}

void *thread_del(void *tid)
{
	return NULL;
}
