#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>
#include <genmc.h>

int main()
{
	void *a = malloc(sizeof(int));
	/* printf("addr: %p\n", a); */

	/* should produce a warning about unfreed memory */
	return 0;
}
