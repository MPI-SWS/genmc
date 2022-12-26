#include <assert.h>

#define K (1 << 31)

unsigned int x = K;

int main()
{
	unsigned int c = K;
	assert(__atomic_compare_exchange_n(&x, &c, 0, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
	return 0;
}
