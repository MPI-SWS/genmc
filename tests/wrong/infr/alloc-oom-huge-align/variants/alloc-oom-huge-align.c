#include <stdlib.h>

int main()
{
	const size_t size = 1;
	const size_t alignment = (size_t)1 << 31;

	void *foo0 = aligned_alloc(size, alignment);
	void *foo1 = aligned_alloc(size, alignment);
	void *foo2 = aligned_alloc(size, alignment);

	return 0;
}
