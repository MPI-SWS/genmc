#include <stdatomic.h>
#include <assert.h>

/* Two 1-byte atomic stores followed by a 2-byte atomic load over the same
 * bytes is a mixed-size access. GenMC must report it; before the fix it
 * wrongly returned 1 (one byte store) instead of detecting the error. */

atomic_ushort A;

int main()
{
	_Atomic unsigned char *a8 = (_Atomic unsigned char *)&A;
	atomic_store_explicit(&a8[0], 1, memory_order_seq_cst);
	atomic_store_explicit(&a8[1], 1, memory_order_seq_cst);
	unsigned short val = atomic_load_explicit(&A, memory_order_seq_cst);
	assert(val == 0x0101); /* never reached: load raises a mixed-size error */
	return 0;
}
