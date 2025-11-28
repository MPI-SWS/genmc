#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>

void assert_correct_unsigned() {
	_Atomic(uint8_t) x = 150;
	atomic_fetch_add(&x, 150);
	__atomic_fetch_max((uint8_t *)&x, 200, __ATOMIC_SEQ_CST);

	/* Treating atomics as 64 bit values and not properly sign extending them for unsigned min/max operations
	 * resulted in this incorrectly returning uint8_t(max((150 + 150), 200)) == uint8_t(300) == 44
	 */
	assert(200 == atomic_load(&x));

	atomic_store(&x, 50);
	atomic_fetch_sub(&x, 150); /* uint8_t(50 - 150) --> 156 */
	__atomic_fetch_min((uint8_t *)&x, 200, __ATOMIC_SEQ_CST);

	assert(156 == atomic_load(&x));
}

void assert_correct_signed() {
	_Atomic(int8_t) x = 100;
	atomic_fetch_add(&x, 100);
	__atomic_fetch_max((int8_t *)&x, 50, __ATOMIC_SEQ_CST);

	assert(50 == atomic_load(&x));

	atomic_store(&x, -100);
	atomic_fetch_sub(&x, 100); /* should wrap around 56 */
	__atomic_fetch_min((int8_t *)&x, -50, __ATOMIC_SEQ_CST);

	assert(-50 == atomic_load(&x));
}

int main()
{
	assert_correct_unsigned();
	assert_correct_signed();
	return 0;
}
