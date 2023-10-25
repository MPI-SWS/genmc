#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>

/* Make sure that FAIs on unallocated memory doesn't confuse the interpreter */

int main()
{
	int a = atomic_fetch_add((atomic_int *) 0xdeadbeef, 42);
	return 0;
}
