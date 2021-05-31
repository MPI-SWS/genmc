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
			if (!atomic_compare_exchange_strong(&x, &r, 42))
				continue;
		} else {
			if (!atomic_compare_exchange_strong(&x, &r, 17))
				continue;
		}
	}

	return 0;
}
