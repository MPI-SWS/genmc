#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

struct bar {
	int x;
	int y;
};

struct foo {
	int a[4];
	int b;
	struct bar c;
};

int main()
{
	/*
	 * This will produce a warning for incompatible ptr to int conversion,
	 * so a[0] should have some weird value, while a[1],a[2] and a[3] should
	 * be set to 0.
	 * clang emits llvm.memset only for the .bar part of "w", if the size
	 * of foo.a is small enough
	 */
	struct foo w = { .a = "2222", .b = 42 };

	return 0;
}
