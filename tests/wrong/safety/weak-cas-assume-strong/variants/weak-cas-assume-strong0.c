/* This test demonstrates that treating weak CASes as strong hides bugs. */

#include <assert.h>
#include <stdatomic.h>

static atomic_int x;

int main(void)
{
	int e = 0;
	int r = atomic_compare_exchange_weak_explicit(&x, &e, 1,
						      memory_order_relaxed,
						      memory_order_relaxed);
	assert(r); /* fails when the weak CAS spuriously fails */
	return 0;
}
