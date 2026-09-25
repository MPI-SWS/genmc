#include <pthread.h>
#include <atomic>
#include <cassert>
#include <cstdint>

/* Read-modify-write through atomic_ref on a plain object: both increments
 * must be observed, whichever order they land in. */
std::uint32_t counter = 0;

void incr()
{
	std::atomic_ref<std::uint32_t> c{counter};

	c.fetch_add(1, std::memory_order_relaxed);
}

int main()
{
	pthread_t threads[2];

	for (auto i = 0; i < 2; ++i) {
		pthread_create(
			&threads[i],
			nullptr,
			[](void *) -> void * {incr(); return nullptr;},
			nullptr);
	}
	for (auto i = 0; i < 2; ++i) {
		pthread_join(threads[i], nullptr);
	}

	std::atomic_ref<std::uint32_t> c{counter};
	assert(c.load(std::memory_order_relaxed) == 2);

	return 0;
}
