#include <pthread.h>
#include <atomic>
#include <cassert>

// using namespace std;

std::atomic<int> x(0);
std::atomic<int> y(0);

void t0()
{
	y.store(20, std::memory_order_release);
	x.store(10, std::memory_order_release);
}

void t1()
{
	if (x.load(std::memory_order_acquire) == 10) {
		assert(y.load(std::memory_order_acquire) == 20);
	}
}

int main()
{
	pthread_t threads[2];

	pthread_create(
		&threads[0],
		nullptr,
		[](void*) -> void* {t0(); return nullptr;},
		nullptr);
	pthread_create(
		&threads[1],
		nullptr,
		[](void*) -> void* {t1(); return nullptr;},
		nullptr);
	for (auto i = 0; i < 2; ++i) {
		pthread_join(threads[i], nullptr);
	}

	return 0;
}
