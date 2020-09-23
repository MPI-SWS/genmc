#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

struct foo {
	atomic_int x;
	int y;
	int a[4];
};

int fun(struct foo f)
{
	return f.x + f.y;
}

int main()
{
	struct foo w = { .a = { [0 ... 3] = 42 }, .y = 42, .x = 17 };

	int r = fun(w);

	return 0;
}
