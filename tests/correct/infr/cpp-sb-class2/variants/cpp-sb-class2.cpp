#include <pthread.h>
#include <atomic>

class SB {

public:
	SB() {
		x = new std::atomic<int>(0);
		y = new std::atomic<int>(0);
	}

	~SB() {
		delete x;
		delete y;
	}

	std::atomic<int> *x;
	std::atomic<int> *y;
};

void *thread_1(void *arg)
{
	auto *sb = (SB *) arg;

	sb->x->store(42);
	sb->y->load();
	return nullptr;
}

void *thread_2(void *arg)
{
	auto *sb = (SB *) arg;

	sb->y->store(42);
	sb->x->load();
	return nullptr;
}

int main()
{
	pthread_t t1, t2;

	SB vars;

	pthread_create(&t1, nullptr, thread_1, &vars);
	pthread_create(&t2, nullptr, thread_2, &vars);

	pthread_join(t1, nullptr);
	pthread_join(t2, nullptr);

	return 0;
}
