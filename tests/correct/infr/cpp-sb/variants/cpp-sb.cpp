// #include <stdlib.h>
#include <pthread.h>
#include <atomic>

std::atomic<int> x;
std::atomic<int> y;

void *thread_1(void *unused)
{
	x.store(42);
	int r = y.load();
	return nullptr;
}

void *thread_2(void *unused)
{
	y.store(42);
	int r = x.load();
	return nullptr;
}

int main()
{
	pthread_t t1, t2;

	/* Try out some lambdas as well */
	// pthread_create(&t1, nullptr, [](void *) -> void* { thread_1(); return nullptr; }, nullptr);
	// pthread_create(&t2, nullptr, [](void *) -> void * { thread_2(); return nullptr; }, nullptr);
	pthread_create(&t1, nullptr, thread_1, nullptr);
	pthread_create(&t2, nullptr, thread_2, nullptr);

	return 0;
}
