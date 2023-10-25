#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <genmc.h>

/* Some utilities */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define __stringify_1(x) #x
#define __stringify(x) __stringify_1(x)

#define strlen(s) (sizeof(s)/sizeof(s[0]))

/* Test case parameters */
#define FILENAME foo.txt
#define BACKUPNAME FILENAME~

#define FILE_DATA "data"
#define BUFFER_DATA "modified data"

static const char file[] = __stringify(FILENAME);
static const char backup[] = __stringify(BACKUPNAME);

static const char initial_data[] = FILE_DATA;
static const char buffer_data[] = BUFFER_DATA;

/* Nano code + faked context */
/* #define MAKE_BACKUP 1 // defined in args.*.in */

#include <fake.h>
#include <nano.c>

/* Test case */

void __VERIFIER_recovery_routine(void)
{
	int init_len = strlen(initial_data);
	int buf_len = strlen(buffer_data);
	int max_size = MAX(init_len, buf_len);
	char buf[MAX(init_len, buf_len)];

	int fd = open(__stringify(FILENAME), O_RDONLY, 0640);
	int fb = open(__stringify(BACKUPNAME), O_RDONLY, 0640);
	assert(fd != -1);

	int nr = lseek(fd, 0, SEEK_END);
	/* int nr = read(fd, buf, max_size); */

	/* We wouldn't want to see the file smaller... */
	/* assert(nr >= init_len && nr <= buf_len); */

	/* If we do see the file smaller, there should at least be a full backup */
	if (nr < init_len) {
		/* assert(fb != -1); */
		int nrb = lseek(fb, 0, SEEK_END);
		if (nrb >= 0)
			assert(nrb == init_len);
	}
}

int main()
{
	/* Create an existing file */
	int fd = creat(__stringify(FILENAME), 0640);
	int nw = write(fd, initial_data, strlen(initial_data));
	close(fd);

	/* Assume the file has persisted on disk */
	__VERIFIER_pbarrier();

	/* Fake nano's buffer creation and population */
	fake_make_new_buffer();

	/* Execute nano's writeout */
	bool ret = write_file(file, NULL, FALSE, OVERWRITE, FALSE);
	assert(ret);

	return 0;
}
