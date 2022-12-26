#include <pthread.h>
#include <atomic>

class SB {

public:
	SB() {
		x.store(0);
		y.store(0);
	}

	void doXY() {
		x.store(1);
		int r_y = y.load();
	}

	void doYX() {
		y.store(1);
		int r_x = x.load();
	}

private:
	std::atomic<int> x;
	std::atomic<int> y;
};

SB vars;

void *thread_1(void *unused)
{
	vars.doXY();
	return nullptr;
}

void *thread_2(void *unused)
{
	vars.doYX();
	return nullptr;
}

int main()
{
	pthread_t t1, t2;

	pthread_create(&t1, nullptr, thread_1, nullptr);
	pthread_create(&t2, nullptr, thread_2, nullptr);

	return 0;
}
