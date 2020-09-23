#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

struct bar {
	int x;
	int y;
};

struct foo {
	int b;
	struct bar c;
	int a[5];
};

int main()
{
	struct foo w = { .a = { [0 ... 2] = 42 }, .b = 42 };

	return 0;
}
