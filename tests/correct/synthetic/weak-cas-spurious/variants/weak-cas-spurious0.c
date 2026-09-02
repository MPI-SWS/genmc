/* Observable spurious failure of a weak CAS (not used in a loop). */

#include <stdatomic.h>

static atomic_int x;

int main(void)
{
	int e = 0;
	int r = atomic_compare_exchange_weak_explicit(&x, &e, 1,
						      memory_order_relaxed,
						      memory_order_relaxed);
	/* r == 1 with x == 1 (success), or r == 0 with x == 0 (spurious) */
	return r;
}
