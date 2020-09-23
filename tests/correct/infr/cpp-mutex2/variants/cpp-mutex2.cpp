#include <pthread.h>
#include <cassert>

class mutex {
	pthread_mutex_t *_lock;
public:
        mutex(pthread_mutex_t *lock) : _lock(lock) {
		pthread_mutex_lock(_lock);
        }
        ~mutex() {
		pthread_mutex_unlock(_lock);
        }
};

int count = 0;

void t0(void *l)
{
	mutex lock((pthread_mutex_t *) l);
	count++;;
	return;
}

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main()
{
	pthread_t threads[2];

	auto ret = pthread_create(&threads[0],
				  nullptr,
				  [](void *) -> void* { t0((void *) &lock); return nullptr; },
				  nullptr);
	assert(ret == 0);
	ret = pthread_create(&threads[1],
				  nullptr,
				  [](void *) -> void* { t0((void *) &lock); return nullptr; },
				  nullptr);
	assert(ret == 0);

	ret = pthread_join(threads[0], nullptr);
	assert(ret == 0);
	ret = pthread_join(threads[1], nullptr);
	assert(ret == 0);

	assert(count == 2);
	return 0;
}
