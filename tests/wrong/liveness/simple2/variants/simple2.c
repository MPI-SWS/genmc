#include <stdatomic.h>
#include <pthread.h>

atomic_int x;

int main()
{
	while (!atomic_exchange(&x, 0))
		;

	return 0;
}
