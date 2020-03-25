#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>

#include "../gcd.c"

int main()
{
	// for testing with small unwinding bounds
	unsigned a_in = 8; // __VERIFIER_nondet_uint(); //=8;
	unsigned b_in = 6; // __VERIFIER_nondet_uint(); //=6;

	__VERIFIER_assume(a_in > 0);
	__VERIFIER_assume(b_in > 0);

	start(a_in, b_in);
	/* check_gcd(a_in, b_in, start(a_in, b_in)); */
	return 0;
}
