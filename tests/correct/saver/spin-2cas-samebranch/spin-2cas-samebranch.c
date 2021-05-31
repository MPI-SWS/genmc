atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
	for (int i = 1u; i <= 42; i++) {
		x = i;
		y = i;
	}
	return NULL;
}

void *thread_2(void *unused)
{
	int r_x = 42;
	int r_y = 42;

	while (!atomic_compare_exchange_strong(&x, &r_x, 17) &&
	       !atomic_compare_exchange_strong(&y, &r_y, 17)) {
		r_x = 42;
		r_y = 42;
	}

	/* int success_x = 0; */
	/* int success_y = 0; */

	/* while (!success_x && !success_y) { */
	/* 	int r_x = 42; */
	/* 	int r_y = 42; */

	/* 	success_x = atomic_compare_exchange_strong(&x, &r_x, 17); */
	/* 	success_y = atomic_compare_exchange_strong(&y, &r_y, 17);		 */
	/* }	 */
	return NULL;
}
