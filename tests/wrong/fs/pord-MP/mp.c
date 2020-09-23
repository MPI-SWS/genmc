void *thread_1(void *unused)
{
	char buf[8];
	buf[0] = 42;

	int fd_x = open("x", O_WRONLY, S_IRWXU);
	int fd_y = open("y", O_WRONLY, S_IRWXU);

	write(fd_x, buf, 1);
	/* sync(); */
	write(fd_y, buf, 1);

	return NULL;
}

void __VERIFIER_recovery_routine(void)
{
	char buf_x[8], buf_y[8];

	int fd_y = open("y", O_RDONLY, S_IRWXU);
	int fd_x = open("x", O_RDONLY, S_IRWXU);

	read(fd_y, buf_y, 1);
	read(fd_x, buf_x, 1);

	assert(!(buf_y[0] == 42 && buf_x[0] == 1));
	return;
}
