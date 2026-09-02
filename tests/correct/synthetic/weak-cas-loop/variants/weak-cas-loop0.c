/* Lock-free counter using a weak-CAS retry loop. Can be
 * automatically strengthened to a strong CAS.
 */

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>

static atomic_int ctr;

static void incr(void)
{
	int cur = atomic_load_explicit(&ctr, memory_order_relaxed);
	while (!atomic_compare_exchange_weak_explicit(&ctr, &cur, cur + 1,
						      memory_order_relaxed,
						      memory_order_relaxed))
		; /* on failure, cur is updated with the current value */
}

static void *run(void *arg)
{
	(void)arg;
	incr();
	return NULL;
}

int main(void)
{
	pthread_t t;

	pthread_create(&t, NULL, run, NULL);
	incr();
	pthread_join(t, NULL);

	assert(atomic_load_explicit(&ctr, memory_order_relaxed) == 2);
	return 0;
}
