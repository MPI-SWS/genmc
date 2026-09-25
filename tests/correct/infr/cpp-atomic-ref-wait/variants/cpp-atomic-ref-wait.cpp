#include <pthread.h>
#include <atomic>
#include <cassert>
#include <cstdint>

/* wait() must not return until `flag` moves off the awaited value, and the
 * acquire ordering it is given must make the released `data` visible.
 * notify_one() is a no-op under the polling implementation. */
std::uint32_t data = 0;
std::uint32_t flag = 0;

void t0()
{
	std::atomic_ref<std::uint32_t> f{flag};

	data = 42;
	f.store(1, std::memory_order_release);
	f.notify_one();
}

void t1()
{
	std::atomic_ref<std::uint32_t> f{flag};

	f.wait(0, std::memory_order_acquire);
	assert(data == 42);
}

int main()
{
	pthread_t threads[2];

	pthread_create(
		&threads[0],
		nullptr,
		[](void *) -> void * {t0(); return nullptr;},
		nullptr);
	pthread_create(
		&threads[1],
		nullptr,
		[](void *) -> void * {t1(); return nullptr;},
		nullptr);
	for (auto i = 0; i < 2; ++i) {
		pthread_join(threads[i], nullptr);
	}

	return 0;
}
