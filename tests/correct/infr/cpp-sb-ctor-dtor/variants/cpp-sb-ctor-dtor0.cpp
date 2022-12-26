#include <atomic>
// #include <stdio.h>
// #include <stdlib.h>
#include <pthread.h>
#include <genmc.h>

struct StoreLoad {

public:
	StoreLoad(std::atomic<int> &ar, std::atomic<int> &br) : a(ar), b(br) {
		a.store(1, std::memory_order_relaxed);
	}

	~StoreLoad() {
		b.load(std::memory_order_relaxed);
	}

private:
	std::atomic<int> &a;
	std::atomic<int> &b;
};

std::atomic<int> x;
std::atomic<int> y;

void *thread_1(void *unused)
{
	StoreLoad(x, y);
	return NULL;
}

void *thread_2(void *unused)
{
	StoreLoad(y, x);
	return NULL;
}

int main()
{
	pthread_t t1, t2;

	pthread_create(&t1, NULL, thread_1, NULL);
	pthread_create(&t2, NULL, thread_2, NULL);

	return 0;
}
