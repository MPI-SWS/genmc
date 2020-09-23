#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <genmc.h>
#include <assert.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>

void __VERIFIER_recovery_routine(void)
{
	int fd = creat("foo", S_IRWXU);
	/* assert(0); */
	return;
}

int main()
{
	int fd = creat("foo", S_IRWXU);
	return 0;
}
