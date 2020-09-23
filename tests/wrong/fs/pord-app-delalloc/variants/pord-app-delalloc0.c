#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>

void __VERIFIER_recovery_routine(void)
{
	char buf[8];
	buf[0] = 0;
	buf[1] = 0;

	int fd = open("foo", O_RDONLY, 0666);
	assert(fd != -1);

	int nr = pread(fd, buf, 2, 6);
	if (nr < 2)
		return;

	/* Is is possible to read other than {"11", "22"} at offset 6 ? */
	assert(!(buf[0] == 0 || buf[1] == 0));
	return;
}

int main()
{
	char buf1[6] = "111111";
	char buf2[6] = "222222";

	int fd = open("foo", O_CREAT|O_TRUNC|O_RDWR, 0640);

	/* Make the file contain 1111 11 */
	write(fd, buf1, 6);

	__VERIFIER_pbarrier();

	/* Append so that the file contains 1122 2222 */
	pwrite(fd, buf2, 6, 2);
	close(fd);

	return 0;
}
