#include <pthread.h>
#include <atomic>
#include <cstdint>

/* Relaxed accesses through an atomic_ref order nothing, so the plain accesses
 * to `data` race and the checker has to say so. */
std::uint32_t data = 0;
std::uint32_t flag = 0;

void *thread_1(void *unused)
{
	std::atomic_ref<std::uint32_t> f{flag};

	data = 1;
	f.store(1, std::memory_order_relaxed);
	return nullptr;
}

void *thread_2(void *unused)
{
	std::atomic_ref<std::uint32_t> f{flag};
	std::uint32_t r_data = 0;

	if (f.load(std::memory_order_relaxed))
		r_data = data;
	return nullptr;
}

int main()
{
	pthread_t t1, t2;

	pthread_create(&t1, nullptr, thread_1, nullptr);
	pthread_create(&t2, nullptr, thread_2, nullptr);

	return 0;
}
