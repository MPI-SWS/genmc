/* Test case by: Lilith (Github #18) */
#include <stdlib.h>

typedef struct {
	int x;
	int y;
} obj;

int main(void)
{
	obj *o = malloc(sizeof(obj));

	o->x = 1;
	o->y = 2;
	free(o);

	/* should fire an error */
	o->y = 10;

	return 0;
}
