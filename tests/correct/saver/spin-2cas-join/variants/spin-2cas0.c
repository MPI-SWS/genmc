#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>

atomic_int x;
atomic_int y;

int main()
{
	int r, b;

	r = atomic_load_explicit(&x, memory_order_seq_cst);
	for (;;) {
		if (r == 42) {
			if (atomic_compare_exchange_strong(&x, &r, 42))
				break;
		} else {
			if (atomic_compare_exchange_strong(&x, &r, 17))
				break;
		}
		b = r + 1;
		if (b == 17)
			break;
	}

	return 0;
}
