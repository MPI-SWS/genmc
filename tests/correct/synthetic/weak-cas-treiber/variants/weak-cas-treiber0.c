/* Treiber-stack push using a weak-CAS. The threads use statically-allocated
 * nodes so that spin-assume (and, by extension, CAS strengthening) fails. */

#include <pthread.h>
#include <stdatomic.h>

struct node {
	int val;
	struct node *next;
};

static _Atomic(struct node *) top;
static struct node n1, n2;

static void push(struct node *n)
{
	struct node *old = atomic_load_explicit(&top, memory_order_relaxed);
	do {
		n->next = old;
	} while (!atomic_compare_exchange_weak_explicit(
		&top, &old, n, memory_order_release, memory_order_relaxed));
}

static void *run(void *arg)
{
	push((struct node *)arg);
	return NULL;
}

int main(void)
{
	pthread_t t;

	n1.val = 1;
	n2.val = 2;
	pthread_create(&t, NULL, run, &n1);
	push(&n2);
	pthread_join(t, NULL);
	return 0;
}
