#include <stdlib.h>

int main()
{
	unsigned *x = malloc(sizeof(*x));

	/* Bytes 1-3 are never written */
	*(unsigned char *)x = 1;
	unsigned r = *x;
	(void)r;

	free(x);
	return 0;
}
