atomic_int x;
atomic_int y;

pthread_mutex_t l;

void *thread_1(void *ptr)
{
	for (int i = 0; i < 2; i++) {
		pthread_mutex_lock(&l);
		x = 0;
		pthread_mutex_unlock(&l);
		if (x > 0) {
			y++;
			x = 2;
		}
	}
	return NULL;
}

void *thread_2(void *ptr)
{
	if (x > 1) {
		if (y == 3) {
			/* assert(0); */
			/* printf("*****error*****\n"); */
			;
		} else {
			y = 2;
		}
	}
	return NULL;
}

void *thread_3(void *unused)
{
	for (int i = 0; i < 2; i++) {
		pthread_mutex_lock(&l);
		x = 1;
		y = 1;
		pthread_mutex_unlock(&l);
	}
	return NULL;
}
