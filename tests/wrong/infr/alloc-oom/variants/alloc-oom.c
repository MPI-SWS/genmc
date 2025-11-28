#include <stdlib.h>

int main()
{
	const size_t size = (size_t)1 << 31;

	void *foo0 = malloc(size);
	void *foo1 = malloc(size);
	void *foo2 = malloc(size);

	return 0;
}
