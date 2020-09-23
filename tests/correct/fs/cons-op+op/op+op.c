void __VERIFIER_recovery_routine(void)
{
	/* printf("Nothing to do\n"); */
	return;
}

void *thread_1(void *unused)
{
	int fd = open("foo", O_RDONLY, S_IRWXU);
	assert(fd != -1);
	return NULL;
}

void *thread_2(void *unused)
{
	int fd = open("foo", O_RDONLY, S_IRWXU);
	assert(fd != -1);
	return NULL;
}
