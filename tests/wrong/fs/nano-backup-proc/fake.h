/*
 * "Fake" declarations to scaffold nano's environment.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * Author: Michalis Kokologiannakis <michalis@mpi-sws.org>
 */

#ifndef __FAKE_H
#define __FAKE_H

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

/*************************************************************
 * Definitions taken from nano's code
 *************************************************************/

typedef enum {
	NIX_FILE, DOS_FILE, MAC_FILE
} format_type;

typedef enum {
	VACUUM, HUSH, NOTICE, MILD, ALERT
} message_type;

typedef enum {
	OVERWRITE, APPEND, PREPEND
} kind_of_writing_type;

typedef enum {
	SOFTMARK, HARDMARK
} mark_type;

typedef enum {
	CENTERING, FLOWING, STATIONARY
} update_type;

typedef enum {
	ADD, ENTER, BACK, DEL, JOIN, REPLACE,
#ifdef ENABLE_WRAPPING
	SPLIT_BEGIN, SPLIT_END,
#endif
	INDENT, UNINDENT,
#ifdef ENABLE_COMMENT
	COMMENT, UNCOMMENT, PREFLIGHT,
#endif
	ZAP, CUT, CUT_TO_EOF, COPY, PASTE, INSERT,
	COUPLE_BEGIN, COUPLE_END, OTHER
} undo_type;

/* More structure types. */
struct undostruct;
typedef struct undostruct undostruct;

typedef struct linestruct {
	char *data;
		/* The text of this line. */
	ssize_t lineno;
		/* The number of this line. */
#ifndef NANO_TINY
	ssize_t extrarows;
		/* The extra rows that this line occupies when softwrapping. */
#endif
	struct linestruct *next;
		/* Next node. */
	struct linestruct *prev;
		/* Previous node. */
#ifdef ENABLE_COLOR
	short *multidata;
		/* Array of which multi-line regexes apply to this line. */
#endif
#ifndef NANO_TINY
	bool has_anchor;
		/* Whether the user has placed an anchor at this line. */
#endif
} linestruct;

typedef struct openfilestruct {
	char *filename;
		/* The file's name. */
	linestruct *filetop;
		/* The file's first line. */
	linestruct *filebot;
		/* The file's last line. */
	linestruct *edittop;
		/* The current top of the edit window for this file. */
	linestruct *current;
		/* The current line for this file. */
	size_t totsize;
		/* The file's total number of characters. */
	size_t firstcolumn;
		/* The starting column of the top line of the edit window.
		 * When not in softwrap mode, it's always zero. */
	size_t current_x;
		/* The file's x-coordinate position. */
	size_t placewewant;
		/* The file's x position we would like. */
	ssize_t current_y;
		/* The file's y-coordinate position. */
	struct stat *statinfo;
		/* The file's stat information from when it was opened or last saved. */
#ifdef ENABLE_WRAPPING
	linestruct *spillage_line;
		/* The line for prepending stuff to during automatic hard-wrapping. */
#endif
#ifndef NANO_TINY
	linestruct *mark;
		/* The line in the file where the mark is set; NULL if not set. */
	size_t mark_x;
		/* The mark's x position in the above line. */
	mark_type kind_of_mark;
		/* Whether it is a soft (with Shift) or a hard mark. */
	format_type fmt;
		/* The file's format -- Unix or DOS or Mac. */
	char *lock_filename;
		/* The path of the lockfile, if we created one. */
	undostruct *undotop;
		/* The top of the undo list. */
	undostruct *current_undo;
		/* The current (i.e. next) level of undo. */
	undostruct *last_saved;
		/* The undo item at which the file was last saved. */
	undo_type last_action;
		/* The type of the last action the user performed. */
#endif
	bool modified;
		/* Whether the file has been modified. */
#ifdef ENABLE_COLOR
	syntaxtype *syntax;
		/* The syntax that applies to this file, if any. */
#endif
#ifdef ENABLE_MULTIBUFFER
	struct openfilestruct *next;
		/* The next open file, if any. */
	struct openfilestruct *prev;
		/* The preceding open file, if any. */
#endif
} openfilestruct;


/*************************************************************
 * Definitions to fake some missing libc functionality
 *************************************************************/

typedef long dev_t;
typedef long ino_t;
typedef long nlink_t;
typedef long uid_t;
typedef long gid_t;
typedef long blksize_t;
typedef long blkcnt_t;
typedef long time_t;

/* struct timespec { */
/*    time_t   tv_sec; */
/*    long     tv_nsec; */
/* } */

struct stat {
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;
	time_t st_atime;
	time_t st_mtime;
	time_t st_ctime;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
	mode_t st_attr;
};


/* For our purposes, just make stat zero-initialize a statbuf */
int stat(const char *name, struct stat *statbuf)
{
	statbuf->st_dev = 0;
	statbuf->st_ino = 0;
	statbuf->st_mode = 0;
	statbuf->st_nlink = 0;
	statbuf->st_uid = 0;
	statbuf->st_gid = 0;
	statbuf->st_rdev = 0;
	statbuf->st_size = 0;
	statbuf->st_atime = 0;
	statbuf->st_mtime = 0;
	statbuf->st_ctime = 0;
	statbuf->st_blksize = 0;
	statbuf->st_blocks = 0;
	statbuf->st_attr = 0;
	return 0;
}
#define stat_with_alloc(name, st) do {} while (0)

#define MAX_FD_NUM 20
int __fds[MAX_FD_NUM];
int __next_fd;

#define fopen(name, mode)			\
({					        \
	/* Cheat by always opening for RW */    \
	int fd = open(name, O_RDWR, 0640);	\
	assert(__next_fd < MAX_FD_NUM);		\
	__fds[__next_fd] = fd;			\
	(FILE *) &__fds[__next_fd++];		\
})

FILE *fdopen(int fd, const char *mode)
{
	assert(__next_fd < MAX_FD_NUM);
	__fds[__next_fd] = fd;
	return (FILE *) &__fds[__next_fd++];
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	int nr = read(*((int *) stream), ptr, size * nmemb);
	/* size_t ret =  */
	return (nr == -1) ? 0 : nr;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	int nw = write(*((int *) stream), ptr, size * nmemb);
	return (nw == -1) ? 0 : nw;
}

int fflush(FILE *stream)
{
	return 0;
}

int fclose(FILE *stream)
{
	return close(*((int *) stream));
}

int ferror(FILE *stream)
{
	return 0;
}

#define EOF -1

#define putc(...) 42 /* assume success */

/* This hack will only work for this test case -- charalloc is used
 * to allocated a buffer for the backup's name, and sprintf is used to create it*/
#ifndef BACKUPNAME
#error "The name of the backup must be defined!\n"
#endif

#define charalloc(...) __stringify(BACKUPNAME)
#define sprintf(...) do {} while (0)

/* Some functions that should not be called */
#define strcmp(...) ({ assert(0); 0; })
#define mallocstrcpy(...) ({ assert(0); NULL; })

/* We replaced all dynamic allocations, so make free() do nothing */
#define free(x) do {} while (0)

/*************************************************************
 * "Cheater" definitions based on a restricted nano config
 *************************************************************/

#define INSECURE_BACKUP 1

#undef ENABLE_COLOR
#undef ENABLE_OPERATINGDIR
#undef NANO_TINY

#define TRUE 1
#define FALSE 0

#define S_ISFIFO(f) FALSE
#define RW_FOR_ALL S_IRWXU

/*
 * Getting something that works in C and CPP for an arg that may or may
 * not be defined is tricky.  Here, if we have "#define CONFIG_BOOGER 1"
 * we match on the placeholder define, insert the "0," for arg1 and generate
 * the triplet (0, 1, 0).  Then the last step cherry picks the 2nd arg (a one).
 * When CONFIG_BOOGER is not defined, we generate a (... 1, 0) pair, and when
 * the last step cherry picks the 2nd arg, we get a zero.
 */
#define __ARG_PLACEHOLDER_1 0,
#define config_enabled(cfg) _config_enabled(cfg)
#define _config_enabled(value) __config_enabled(__ARG_PLACEHOLDER_##value)
#define __config_enabled(arg1_or_junk) ___config_enabled(arg1_or_junk 1, 0)
#define ___config_enabled(__ignored, val, ...) val

/*
 * IS_ENABLED(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y' or 'm',
 * 0 otherwise.
 *
 */
#define IS_ENABLED(option)						\
	(config_enabled(option) || config_enabled(option##_MODULE))

#define ISSET(option) IS_ENABLED(option)

#define	statusline(...) do {} while (0)
#define statusbar(...) do {} while (0)
#define warn_and_briefly_pause(...) do {} while (0)
#define user_wants_to_proceed() 1

#define fchmod(...) do {} while (0)
#define fchown(...) do {} while (0)
#define futimens(...) do {} while (0)

#define IGNORE_CALL_RESULT(call) call

#define block_sigwinch(bool) do {} while (0)
#define	install_handler_for_Ctrl_C() do {} while (0)
#define restore_handler_for_Ctrl_C() do {} while (0)
#define recode_LF_to_NUL(...) do {} while (0)
#define recode_NUL_to_LF(...) do {} while (0)
#define find_and_prime_applicable_syntax() do {} while (0)
#define precalc_multicolorinfo() do {} while (0)
#define titlebar(t) do {} while (0)

#define do_lockfile(...) ({ NULL; })
#define delete_lockfile(lock_filename) do {} while (0)

#define get_full_path(name) name
#define real_dir_from_tilde(name) name
#define get_next_filename(name, extension) ""
#define copy_of(name) ""
#define safe_tempfile(target) ""

struct undostruct {};

/* Fake some global variables and functions */

#define BUFSIZ 64

bool have_palette;
bool refresh_needed;
char *backup_dir = NULL;
openfilestruct *openfile = NULL;

#ifndef BUFFER_DATA
#error "Test case parameters need to be defined!"
#endif

linestruct __only_line = { .data = buffer_data };
struct stat __open_file_stat;
openfilestruct __open_file = {
	.filetop = &__only_line,
	.filebot = &__only_line,
	.edittop = &__only_line,
	.current = &__only_line,
	.totsize = strlen(BUFFER_DATA),
	.statinfo = &__open_file_stat,
	.fmt = NIX_FILE
};

void fake_make_new_buffer(void)
{
	/* Since we are only interested in one file, we'll use __open_file
	 * and __only_line instead of dynamically allocating a struct.
	 * We initialize it with the given test case parameters */
	openfile = &__open_file;
	return;
}

#endif /* __FAKE_H__ */
