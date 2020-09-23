#include <pthread.h>
#include <atomic>

std::atomic<int> x;
std::atomic<int> y;

class SB1 {

public:
	SB1() {
		x.store(42);
		val = y.load();
	}

private:
	int val;
};

class SB2 {

public:
	SB2() {
		y.store(42);
		val = x.load();
	}

private:
	int val;
};

void *thread_1(void *unused)
{
	SB1 t1;
	return nullptr;
}

void *thread_2(void *unused)
{
	SB2 t2;
	return nullptr;
}

int main()
{
	pthread_t t1, t2;

	pthread_create(&t1, nullptr, thread_1, nullptr);
	pthread_create(&t2, nullptr, thread_2, nullptr);

	return 0;
}
