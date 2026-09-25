#include <pthread.h>
#include <atomic>
#include <cassert>

/* atomic_flag::test observes the flag without setting it, so the setter's
 * release must be what makes `data` visible. */
std::atomic_flag f = ATOMIC_FLAG_INIT;
int data = 0;

void t0()
{
	data = 42;
	f.test_and_set(std::memory_order_release);
}

void t1()
{
	if (f.test(std::memory_order_acquire))
		assert(data == 42);
}

int main()
{
	pthread_t threads[2];

	assert(!f.test());

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

	assert(std::atomic_flag_test(&f));

	return 0;
}
