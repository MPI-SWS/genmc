#include <pthread.h>
#include <atomic>
#include <cassert>
#include <cstdint>

/* `data` and `flag` stay plain objects with the layout of their own types;
 * only the accesses through atomic_ref are atomic. The release/acquire pair
 * on `flag` must order the plain accesses to `data`. */
std::uint32_t data = 0;
std::uint32_t flag = 0;

void t0()
{
	std::atomic_ref<std::uint32_t> f{flag};

	data = 42;
	f.store(1, std::memory_order_release);
}

void t1()
{
	std::atomic_ref<std::uint32_t> f{flag};

	if (f.load(std::memory_order_acquire) == 1)
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
