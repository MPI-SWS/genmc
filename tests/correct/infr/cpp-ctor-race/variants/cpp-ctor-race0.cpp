#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

int x;
struct Foo {

	Foo() = default;
	~Foo() { x=1;++numFoos; } // use -fno-use-cxa-atexit due to lli bug

	inline static int numFoos = 0; // use -std=c++17
};

Foo f;

void *thread_1(void *unused)
{
	Foo::numFoos = 42;
	return NULL;
}

int main()
{
	pthread_t t1;

	if (pthread_create(&t1, NULL, thread_1, NULL))
		std::abort();

	std::atexit([](void){ return; });

	return 0;
}
