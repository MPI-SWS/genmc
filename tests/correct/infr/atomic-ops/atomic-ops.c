int x = -1;
int y = 42;

void *thread_1(void *unused)
{
	int r_y = y;

	__sync_fetch_and_umax(&x, r_y);
	assert(x == -1);

	__sync_fetch_and_max(&x, r_y);
	assert(x == 42);
	return NULL;
}
