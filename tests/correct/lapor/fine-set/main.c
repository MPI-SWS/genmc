#include <assert.h>
#include "fine-set.c"

/* Driver code */
#ifndef MAX_THREADS
# define MAX_THREADS 32
#endif
#ifndef MAX_FREELIST
# define MAX_FREELIST 32 /* Each thread can own up to MAX_FREELIST free nodes */
#endif

#ifdef CONFIG_SET_ADDERS
# define DEFAULT_ADDERS (CONFIG_SET_ADDERS)
#else
# define DEFAULT_ADDERS 2
#endif
#ifdef CONFIG_SET_SEEKERS
# define DEFAULT_SEEKERS (CONFIG_SET_SEEKERS)
#else
# define DEFAULT_SEEKERS 0
#endif
#ifdef CONFIG_SET_REMOVERS
# define DEFAULT_REMOVERS (CONFIG_SET_REMOVERS)
#else
# define DEFAULT_REMOVERS 0
#endif

static int adders = DEFAULT_ADDERS, seekers = DEFAULT_SEEKERS, removers = DEFAULT_REMOVERS;
static int num_threads;
static struct set_head myset;

pthread_t threads[MAX_THREADS + 1];
int param[MAX_THREADS + 1];
struct set_node free_lists[MAX_THREADS + 1][MAX_FREELIST];
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

struct set_node *new_node(int key, int elem)
{
	int t = get_thread_num();

	assert(free_index[t] < MAX_FREELIST);
	free_lists[t][free_index[t]].key = key;
	free_lists[t][free_index[t]].val = elem;
	return &free_lists[t][free_index[t]++];
}

/* Should be called before threads are spawned (from main()) */
void init()
{
	num_threads = adders + seekers + removers + 1;
	for (int j = 0; j < num_threads; j++)
		param[j] = j;

	set_init(&myset);
	for (int i = 0; i < 8; i++) {
		if (i % 2 == 0)
			add(&myset, i);
	}
}

void *thread_add(void *tid)
{
	int t = (*(int *) tid);
	set_thread_num(t);

	add(&myset, (t * 7) % 12);
	return NULL;
}

void *thread_seek(void *tid)
{
	int t = (*(int *) tid);
	set_thread_num(t);

	contains(&myset, (t * 7) % 12);
	return NULL;
}

void *thread_del(void *arg)
{
	return NULL;
}
