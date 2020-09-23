#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

struct foo {
	int a[8];
	int x;
	long y;
	char z;
};

struct foo f;

int main()
{
	struct foo q = { .x = 42 };

	return 0;
}
