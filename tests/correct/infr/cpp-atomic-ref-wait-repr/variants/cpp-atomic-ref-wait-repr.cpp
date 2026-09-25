#include <atomic>
#include <cassert>

/* wait() compares value representations, which for floating point is not what
 * operator== would say. 0.0 and -0.0 compare equal but are represented
 * differently, so a wait on 0.0 over a -0.0 referent has nothing to wait for
 * and must return. */
double negzero = -0.0;
int reached = 0;

int main()
{
	std::atomic_ref<double> r{negzero};

	r.wait(0.0, std::memory_order_relaxed);
	reached = 1;

	assert(reached == 1);
	assert(r.load(std::memory_order_relaxed) == 0.0); /* still -0.0 */
	return 0;
}
